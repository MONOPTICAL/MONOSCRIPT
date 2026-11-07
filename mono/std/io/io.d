module mono.std.io.io;

extern(C) void echo(const char* str) nothrow
{
    import core.stdc.stdio : printf;
    
    printf("%s\n", str);
}

extern(C) char* input() nothrow
{
    import core.stdc.stdio : fgets;
    import core.stdc.stdio : stdin;
    import core.stdc.stdlib : malloc;

    enum size = 100;
    char* buffer = cast(char*)malloc(size);
    if (buffer is null)
        return null;
    fgets(buffer, size, stdin);
    return buffer;
}