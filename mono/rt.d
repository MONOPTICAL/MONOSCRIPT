module mono.rt;

import core.stdc.stdio : fflush, fprintf, snprintf, stderr, stdout;
import core.stdc.stdlib : exit;

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
    if (line > 0)
        fprintf(stderr, "Runtime Error: %s at line %d\n", message, line);
    else
        fprintf(stderr, "Runtime Error: %s\n", message);
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
