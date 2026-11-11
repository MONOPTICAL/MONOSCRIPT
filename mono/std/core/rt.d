module mono.std.core.rt;

import core.stdc.stdio : fflush, fprintf, snprintf, stderr, stdout;
import core.stdc.stdlib : exit, getenv;
import mono.std.core.abi : MsArray, MsPanicContext, stringFromCString;
import core.sys.posix.unistd : isatty;

extern(C) nothrow @nogc
{
    alias GC_oom_func = void* function(size_t);

    void GC_init();
    void GC_gcollect();
    void GC_set_oom_fn(GC_oom_func);
    void GC_set_free_space_divisor(size_t divisor);
}

// Код выхода при ошибке выполнения: программа может сама вернуть 1, с ним не спутать
enum PANIC_EXIT_CODE = 101;
// Коды ошибок
enum CODE
{
    INVALID = 1, // Invalid argument
    OUT_OF_RANGE = 2, // Value out of range
    NOT_FOUND = 3, // Item not found
    PERMISSION_DENIED = 4, // Permission denied
    EXITS = 5, // Program exits
    IO_ERROR = 6, // Input/output error
    EOF = 7, // End of file
    TIMEOUT = 8 // Operation timed out
}

// line = 0 значит, что строка неизвестна
extern(C) void ms_panic(const char* message, int line) nothrow @nogc
{
    // То что делает ms_panic, это допечатывает что программа не допиздела, и просто делает exit() с кодом 101(PANIC_EXIT_CODE)
    fflush(stdout);
    const bool color = isatty(2) && getenv("NO_COLOR") is null;
    fprintf(stderr, "%sRuntime Error%s: %s", color ? "\033[1;31m".ptr : "".ptr, color ? "\033[0m".ptr : "".ptr, message);
    if (line > 0)
        fprintf(stderr, " at line %d", line);
    fprintf(stderr, "\n");
    exit(PANIC_EXIT_CODE);
}

// Panic and raise functions for error handling
extern(C) void panic(const char* message, MsPanicContext* panic) nothrow @nogc
{
    if (panic is null) { ms_panic(message, 0); return; }
    if (panic.pending) { ms_panic("internal: panic called while another one is pending", 0); return; }
    panic.message = stringFromCString(message);
    panic.line = 0;
    panic.pending = true;
}


// Функция которая поднимает панику с указанным сообщением и контекстом, но не завершает программу сразу.
extern(C) void ms_raise(const char* message, int line, MsPanicContext* panic) nothrow @nogc
{
    if (panic is null) { ms_panic(message, line); return; } // Если контекст паники не передан, вызываем ms_panic сразу, потому что контекст передаётся в каждуб функцию
    if (panic.pending) { ms_panic("internal: panic raised while another one is pending", line); return; } // Если есть уже паника в ожидании, то вызываем сразу панику чтобы не про3бать её
    
    // А если н1хуя из верхнего не выполняется, то просто делаем текущий контекст паники 'ожидающим' чтобы его в следующем raise можно было обработать
    panic.message = stringFromCString(message);
    panic.line = line;
    panic.pending = true;
}

extern(C) void ms_panic_index(long index, int length, int line, MsPanicContext* panic) nothrow @nogc
{
    char[64] message;
    snprintf(message.ptr, message.length, "index %lld out of bounds (len %d)", index, length);
    ms_raise(message.ptr, line, panic);
}

extern(C) int ms_str_len(const char* text) nothrow @nogc
{
    return cast(int) *(cast(const ulong*) text - 1);
}

// Length metadata makes both len(s) and checked s[i] constant time.
extern(C) const(char)* ms_str_at(const char* text, long index, int line, MsPanicContext* panic) nothrow @nogc
{
    const int length = ms_str_len(text);
    if (index < 0 || index >= length) { ms_panic_index(index, length, line, panic); return null; }
    return text + index;
}

// Место ещё под один элемент: при нехватке ёмкость удваивается, данные переезжают
extern(C) void ms_array_reserve(MsArray* array, size_t elementSize, int hasReferences) nothrow @nogc
{
    import core.stdc.string : memcpy;
    if (array.length < array.capacity) return;

    const int capacity = array.capacity < 4 ? 4 : array.capacity * 2;
    void* data = allocate(capacity * elementSize, hasReferences != 0);
    memcpy(data, array.data, array.length * elementSize);
    array.data = data;
    array.capacity = capacity;
}

extern(C) void ms_panic_result(const char* error, int line, MsPanicContext* panic) nothrow @nogc
{
    char[512] message;
    snprintf(message.ptr, message.length, "result has no value: %s", error);
    ms_raise(message.ptr, line, panic);
}

private extern(C) void* ms_out_of_memory(size_t) nothrow @nogc
{
    ms_panic("out of memory", 0);
    return null;
}

// Первое, что делает программа: и через JIT, и в собранном файле
extern(C) void ms_rt_init() nothrow @nogc
{
    GC_init(); // Иницилизирую гарбидж спортика 
    GC_set_oom_fn(&ms_out_of_memory); // Когда пизда памяти то вызывается ms_out_of_memory
                                      // Который в свою очередь просто паникует и возвращает null
    tuneHeap();
}

// Boehm по умолчанию собирает, как только выделено больше трети кучи (делитель 3).
// Программы, которые плодят много короткоживущих объектов при большом живом наборе,
// тогда по сто раз заново размечают одни и те же живые данные: bintree делал 131 сборку.
// Делитель 1 — сборка, когда выделено столько же, сколько вся куча, примерно как GOGC=100 в Go.
// Стартовую кучу не раздуваем: на маленьком живом наборе большая куча только хуже,
// аллокации уходят из кэша и RSS растёт у всех программ (замер: strings 3M -> 72M RSS)
private void tuneHeap() nothrow @nogc
{
    import core.stdc.stdlib : getenv;

    // Переменную окружения Boehm сам читает в GC_init, если её задали, не перебиваем
    if (getenv("GC_FREE_SPACE_DIVISOR") is null)
        GC_set_free_space_divisor(FREE_SPACE_DIVISOR);
}

enum size_t FREE_SPACE_DIVISOR = 1;

private extern(C) nothrow @nogc
{
    import core.sys.posix.sys.types : pthread_t, pthread_attr_t;
    pthread_t pthread_self();
    int pthread_getattr_np(pthread_t, pthread_attr_t*);
    int pthread_attr_getstack(const pthread_attr_t*, void**, size_t*);
    int pthread_attr_destroy(pthread_attr_t*);
}

// Room left for ms_raise and the GC after the limit is hit.
enum STACK_RESERVE = 256 * 1024;

// Function prologues compare the stack pointer with stackLimit, so deep recursion becomes a panic.
// shortcut: glibc-only (pthread_getattr_np); on failure the limit stays null and the check is off.
extern(C) void ms_stack_init(MsPanicContext* panic) nothrow @nogc
{
    pthread_attr_t attr; // Чисто переменная чтобы хранить в ней атрибуты тякущего потока
                         // Какие атрибуты есть у потока?
                         // stack size - размер стека потока
                         // stack base - адрес начала стека потока
                         // 
    if (pthread_getattr_np(pthread_self(), &attr) != 0) return; // Не смогли получить аттрибуты потока, выходим
                                                                // pthread_self() - текущий поток, &attr - куда записать аттрибуты потока
    
    void* low;
    size_t size;
    if (pthread_attr_getstack(&attr, &low, &size) == 0 && size > 2 * STACK_RESERVE)
        panic.stackLimit = cast(char*) low + STACK_RESERVE;
    pthread_attr_destroy(&attr);
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
extern(C) void* ms_map_get(MsMap* map, ulong key, int line, MsPanicContext* panic) nothrow @nogc
{
    const int position = findPosition(map, key, hashKey(map, key));
    if (position < 0) { ms_raise("key not found", line, panic); return null; }
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

extern(C) void ms_map_del(MsMap* map, ulong key, int line, MsPanicContext* panic) nothrow @nogc
{
    const int position = findPosition(map, key, hashKey(map, key));
    if (position < 0) { ms_raise("key not found", line, panic); return; }

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
extern(C) int ms_map_next(MsMap* map, int* position, int version_, int line, ulong* key, MsPanicContext* panic) nothrow @nogc
{
    if (map.version_ != version_) { ms_raise("map changed during iteration", line, panic); return 0; }

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
