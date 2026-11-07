module mono.rt;

import core.stdc.stdio : fflush, fprintf, snprintf, stderr, stdout;
import core.stdc.stdlib : exit, getenv;
import core.sys.posix.unistd : isatty;

extern(C) nothrow @nogc
{
    alias GC_oom_func = void* function(size_t);

    void GC_init();
    void GC_gcollect();
    void GC_set_oom_fn(GC_oom_func);
}

// Код выхода при ошибке выполнения: программа может сама вернуть 1, с ним не спутать
enum PANIC_EXIT_CODE = 101;

// line = 0 значит, что строка неизвестна
extern(C) void ms_panic(const char* message, int line) nothrow @nogc
{
    // Сначала то, что программа успела напечатать, потом ошибка
    fflush(stdout);
    const bool color = isatty(2) && getenv("NO_COLOR") is null;
    fprintf(stderr, "%sRuntime Error%s: %s", color ? "\033[1;31m".ptr : "".ptr, color ? "\033[0m".ptr : "".ptr, message);
    if (line > 0)
        fprintf(stderr, " at line %d", line);
    fprintf(stderr, "\n");
    exit(PANIC_EXIT_CODE);
}

extern(C) void ms_panic_index(long index, int length, int line) nothrow @nogc
{
    char[64] message;
    snprintf(message.ptr, message.length, "index %lld out of bounds (len %d)", index, length);
    ms_panic(message.ptr, line);
}

private extern(C) void* ms_out_of_memory(size_t) nothrow @nogc
{
    ms_panic("out of memory", 0);
    return null;
}

// Первое, что делает программа: и через JIT, и в собранном файле
extern(C) void ms_rt_init() nothrow @nogc
{
    GC_init();
    GC_set_oom_fn(&ms_out_of_memory);
}

extern(C) void gc_collect() nothrow @nogc
{
    GC_gcollect();
}

// --- map ---
// Массив пар в порядке вставки и индекс для поиска. Ключ и значение лежат в ячейках по 8 байт,
// кодген читает и пишет в них нужный тип прямо по адресу

import core.stdc.string : memcpy, memset, strcmp;

extern(C) nothrow @nogc
{
    void* GC_malloc(size_t size);
    void* GC_malloc_atomic(size_t size);
}

private struct MapEntry
{
    ulong key;
    ulong value;
    uint hash;
    int live;
}

private struct MsMap
{
    MapEntry* entries;
    int* index;       // Номер пары, EMPTY или DELETED
    int used;         // Сколько пар занято, вместе с удалёнными
    int capacity;
    int count;        // Сколько живых пар
    int indexSize;    // Степень двойки
    int version_;     // Меняется, когда меняется набор ключей
    int keyIsString;
    int hasReferences; // Иначе память пар сборщику просматривать незачем
}

private enum EMPTY = -1;
private enum DELETED = -2;

private void* allocate(size_t size, bool hasReferences) nothrow @nogc
{
    return hasReferences ? GC_malloc(size) : GC_malloc_atomic(size);
}

private uint hashKey(const MsMap* map, ulong key) nothrow @nogc
{
    if (map.keyIsString)
    {
        // FNV-1a по содержимому: "a" + "b" должно найти ключ "ab"
        uint hash = 2166136261u;
        for (auto c = cast(const(ubyte)*) key; *c != 0; c++)
        {
            hash ^= *c;
            hash *= 16777619u;
        }
        return hash;
    }

    key ^= key >> 33;
    key *= 0xff51afd7ed558ccdUL;
    key ^= key >> 33;
    return cast(uint) key;
}

private bool sameKey(const MsMap* map, ulong a, ulong b) nothrow @nogc
{
    if (map.keyIsString)
        return strcmp(cast(const char*) a, cast(const char*) b) == 0;
    return a == b;
}

// Позиция в индексе, где лежит ключ. Если ключа нет, вернёт -1
private int findPosition(const MsMap* map, ulong key, uint hash) nothrow @nogc
{
    if (map.indexSize == 0) return -1;

    const uint mask = map.indexSize - 1;
    for (uint position = hash & mask;; position = (position + 1) & mask)
    {
        const int slot = map.index[position];
        if (slot == EMPTY) return -1;
        if (slot != DELETED && map.entries[slot].hash == hash && sameKey(map, map.entries[slot].key, key))
            return position;
    }
}

// Удалённые пары выбрасываются. Индекс вдвое больше числа пар, поэтому в нём всегда есть EMPTY и поиск заканчивается
private void rebuild(MsMap* map) nothrow @nogc
{
    int capacity = map.count * 2 < 8 ? 8 : map.count * 2;
    int indexSize = 16;
    while (indexSize < capacity * 2) indexSize *= 2;

    auto entries = cast(MapEntry*) allocate(capacity * MapEntry.sizeof, map.hasReferences != 0);
    auto index = cast(int*) GC_malloc_atomic(indexSize * int.sizeof);
    memset(index, 0xff, indexSize * int.sizeof); // Все ячейки EMPTY

    int used = 0;
    const uint mask = indexSize - 1;
    for (int i = 0; i < map.used; i++)
    {
        if (!map.entries[i].live) continue;
        entries[used] = map.entries[i];

        uint position = entries[used].hash & mask;
        while (index[position] != EMPTY) position = (position + 1) & mask;
        index[position] = used;
        used++;
    }

    map.entries = entries;
    map.index = index;
    map.used = used;
    map.capacity = capacity;
    map.indexSize = indexSize;
}

extern(C) void* ms_map_new(int keyIsString, int hasReferences) nothrow @nogc
{
    auto map = cast(MsMap*) GC_malloc(MsMap.sizeof);
    map.keyIsString = keyIsString;
    map.hasReferences = hasReferences;
    return map;
}

// Адрес значения. Ключа нет: программа останавливается
extern(C) void* ms_map_get(MsMap* map, ulong key, int line) nothrow @nogc
{
    const int position = findPosition(map, key, hashKey(map, key));
    if (position < 0) ms_panic("key not found", line);
    return &map.entries[map.index[position]].value;
}

// Адрес значения для записи. Ключа нет: он создаётся
extern(C) void* ms_map_insert(MsMap* map, ulong key) nothrow @nogc
{
    const uint hash = hashKey(map, key);
    const int position = findPosition(map, key, hash);
    if (position >= 0) return &map.entries[map.index[position]].value;

    if (map.used == map.capacity) rebuild(map);

    const int slot = map.used++;
    map.entries[slot] = MapEntry(key, 0, hash, 1);
    map.count++;
    map.version_++;

    // Удалённую ячейку можно занять: ключа в таблице точно нет
    const uint mask = map.indexSize - 1;
    uint free = hash & mask;
    while (map.index[free] >= 0) free = (free + 1) & mask;
    map.index[free] = slot;

    return &map.entries[slot].value;
}

extern(C) int ms_map_has(MsMap* map, ulong key) nothrow @nogc
{
    return findPosition(map, key, hashKey(map, key)) >= 0;
}

extern(C) void ms_map_del(MsMap* map, ulong key, int line) nothrow @nogc
{
    const int position = findPosition(map, key, hashKey(map, key));
    if (position < 0) ms_panic("key not found", line);

    const int slot = map.index[position];
    // Обнуляем, чтобы сборщик не держал удалённые объекты
    map.entries[slot] = MapEntry(0, 0, 0, 0);
    map.index[position] = DELETED;
    map.count--;
    map.version_++;
}

extern(C) int ms_map_len(MsMap* map) nothrow @nogc
{
    return map.count;
}

extern(C) int ms_map_version(MsMap* map) nothrow @nogc
{
    return map.version_;
}

// Следующий ключ в порядке вставки. Вернёт 0, когда ключи кончились
extern(C) int ms_map_next(MsMap* map, int* position, int version_, int line, ulong* key) nothrow @nogc
{
    if (map.version_ != version_) ms_panic("map changed during iteration", line);

    while (*position < map.used)
    {
        auto entry = &map.entries[(*position)++];
        if (entry.live)
        {
            *key = entry.key;
            return 1;
        }
    }
    return 0;
}

extern(C) void* ms_map_copy(MsMap* source) nothrow @nogc
{
    auto map = cast(MsMap*) GC_malloc(MsMap.sizeof);
    *map = *source;
    map.version_ = 0;

    if (source.capacity > 0)
    {
        map.entries = cast(MapEntry*) allocate(source.capacity * MapEntry.sizeof, source.hasReferences != 0);
        map.index = cast(int*) GC_malloc_atomic(source.indexSize * int.sizeof);
        memcpy(map.entries, source.entries, source.used * MapEntry.sizeof);
        memcpy(map.index, source.index, source.indexSize * int.sizeof);
    }
    return map;
}
