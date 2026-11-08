module mono.std.io.io;

import core.stdc.stdio : FILE, stdin;
import mono.std.core.abi : GC_malloc_atomic, stringFromBytes, MsPanicContext, MsResult, resultOk, resultErr;
import mono.std.core.rt : CODE;

extern(C) void echo(const char* str) nothrow
{
    import core.stdc.stdio : printf;

    printf("%s\n", str);
}

extern(C) void print(const char* str) nothrow
{
    import core.stdc.stdio : printf;

    printf("%s", str);
}

extern(C) void eecho(const char* str) nothrow
{
    import core.stdc.stdio : fprintf, stderr;

    fprintf(stderr, "%s\n", str);
}

MsResult!(char*)* readline(FILE* file, MsPanicContext* panic) nothrow @nogc
{
    import core.stdc.stdio : fgetc, ungetc, EOF;
    import core.stdc.string : memcpy;

    size_t capacity = 64;
    size_t length = 0;
    char* buffer = cast(char*) GC_malloc_atomic(capacity);

    while (true)
    {
        int c = fgetc(file);
        if (c == EOF || c == '\n')
            break;
        if (c == '\r')
        {
            int next = fgetc(file);
            if (next != '\n' && next != EOF)
                ungetc(next, file);
            break;
        }
        if (length + 1 >= capacity)
        {
            capacity *= 2;
            char* grown = cast(char*) GC_malloc_atomic(capacity);
            memcpy(grown, buffer, length);
            buffer = grown;
        }
        // if capacity is more than 64kb, than return an error
        if (capacity > 64 * 1024)
            return resultErr!(char*)(CODE.OUT_OF_RANGE, "input line is too long");
        buffer[length++] = cast(char) c;
    }

    buffer[length] = 0;
    return resultOk!(char*)(stringFromBytes(buffer, length, panic));
}

/*
Надо будет сделать перегрузку на readlineMax потому что ясен красен 64КБ не хватит для некоторых операций
*/

extern(C) MsResult!(char*)* input(MsPanicContext* panic) nothrow
{
    return readline(stdin, panic);
}
