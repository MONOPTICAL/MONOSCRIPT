module mono.m_test;

extern(C) int sum(const int a, const int b) nothrow @nogc
{
    return a + b;
}

extern(C) int multiply(const int a, const int b) nothrow @nogc
{
    return a * b;
}

extern(C) int subtract(const int a, const int b) nothrow @nogc {
    return a - b;
}

extern(C) int divide(const int a, const int b) nothrow @nogc {
    return b == 0 ? 0 : a / b;
}

extern(C) void echo(const char* str) nothrow
{
    import core.stdc.stdio : printf;
    
    printf("%s\n", str);
}

extern(C) void test() nothrow @nogc
{
    import core.stdc.stdio : printf;

    printf("Hello from D!\n");
}