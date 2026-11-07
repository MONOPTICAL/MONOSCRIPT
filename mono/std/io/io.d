module mono.std.io.io;

import core.stdc.stdio : FILE, stdin;
import mono.std.core.abi : GC_malloc_atomic, stringFromBytes;

extern(C) void echo(const char* str) nothrow
{
    import core.stdc.stdio : printf;

    printf("%s\n", str);
}

char* readline(FILE* file) nothrow @nogc
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
        buffer[length++] = cast(char) c;
    }

    buffer[length] = 0;
    return stringFromBytes(buffer, length);
}

extern(C) char* input() nothrow
{
    return readline(stdin);
}