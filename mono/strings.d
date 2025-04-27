module mono.strings;

extern(C) char* scat(const char* a, const char* b) nothrow @nogc
{
    import core.stdc.string : strlen, strcpy, strcat;
    import core.stdc.stdlib : malloc;

    size_t len_a = strlen(a);
    size_t len_b = strlen(b);

    char* result = cast(char*)malloc(len_a + len_b + 1);
    if (result is null) return null;

    strcpy(result, a);
    strcat(result, b);

    return result;
}

extern(C) char* toString_int(const int a) nothrow @nogc
{
    import core.stdc.stdio : sprintf;
    import core.stdc.string : strlen, strcpy;
    import core.stdc.stdlib : malloc;

    char[20] buffer;
    sprintf(buffer.ptr, "%d", a);

    size_t len_a = strlen(buffer.ptr);

    char* result = cast(char*)malloc(len_a + 1);
    if (result is null) return null;

    strcpy(result, buffer.ptr);

    return result;
}

extern(C) char* toString_bool(const bool a) nothrow @nogc
{
    import core.stdc.string : strlen, strcpy;
    import core.stdc.stdlib : malloc;

    const char* str = a ? "true" : "false";
    size_t len_a = strlen(str);

    char* result = cast(char*)malloc(len_a + 1);
    if (result is null) return null;

    strcpy(result, str);

    return result;
}

extern(C) char* toString_float(const float a) nothrow @nogc
{
    import core.stdc.stdio : sprintf;
    import core.stdc.string : strlen, strcpy;
    import core.stdc.stdlib : malloc;

    char[32] buffer;
    sprintf(buffer.ptr, "%f", a);

    size_t len_a = strlen(buffer.ptr);

    char* result = cast(char*)malloc(len_a + 1);
    if (result is null) return null;

    strcpy(result, buffer.ptr);

    return result;
}