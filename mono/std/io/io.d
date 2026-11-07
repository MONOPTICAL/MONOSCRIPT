module mono.std.io.io;

extern(C) void echo(const char* str) nothrow
{
    import core.stdc.stdio : printf;
    
    printf("%s\n", str);
}
