module mono.strings;

extern(C) nothrow @nogc
{
    void* GC_malloc_atomic(size_t size);
}

extern(C) char* scat(const char* a, const char* b) nothrow @nogc
{
    import core.stdc.string : strlen, strcpy, strcat;

    size_t len_a = strlen(a);
    size_t len_b = strlen(b);

    char* result = cast(char*)GC_malloc_atomic(len_a + len_b + 1);

    strcpy(result, a);
    strcat(result, b);

    return result;
}

extern(C) char* toString_int(const int a) nothrow @nogc
{
    import core.stdc.stdio : sprintf;
    import core.stdc.string : strlen, strcpy;

    char[20] buffer;
    sprintf(buffer.ptr, "%d", a);

    size_t len_a = strlen(buffer.ptr);

    char* result = cast(char*)GC_malloc_atomic(len_a + 1);

    strcpy(result, buffer.ptr);

    return result;
}

extern(C) char* toString_long(const long a) nothrow @nogc
{
    import core.stdc.stdio : sprintf;
    import core.stdc.string : strlen, strcpy;

    char[32] buffer;
    sprintf(buffer.ptr, "%ld", a);

    size_t len_a = strlen(buffer.ptr);

    char* result = cast(char*)GC_malloc_atomic(len_a + 1);

    strcpy(result, buffer.ptr);

    return result;
}

extern(C) char* toString_bool(const bool a) nothrow @nogc
{
    import core.stdc.string : strlen, strcpy;

    const char* str = a ? "true" : "false";
    size_t len_a = strlen(str);

    char* result = cast(char*)GC_malloc_atomic(len_a + 1);

    strcpy(result, str);

    return result;
}

extern(C) char* toString_float(const float a) nothrow @nogc
{
    import core.stdc.stdio : sprintf;
    import core.stdc.string : strlen, strcpy;

    char[32] buffer;
    sprintf(buffer.ptr, "%f", a);

    size_t len_a = strlen(buffer.ptr);

    char* result = cast(char*)GC_malloc_atomic(len_a + 1);

    strcpy(result, buffer.ptr);

    return result;
}

// Самая короткая запись, которая читается обратно в то же число: 0.1, а не 0.100000 и не 0.10000000000000001.
// Целое значение пишем с .0, чтобы было видно, что это не i64
extern(C) char* toString_f64(const double a) nothrow @nogc
{
    import core.stdc.stdio : snprintf;
    import core.stdc.stdlib : strtod;
    import core.stdc.string : strlen, strcpy, strpbrk;

    char[40] buffer;
    foreach (precision; 15 .. 18)
    {
        snprintf(buffer.ptr, buffer.length, "%.*g", precision, a);
        if (strtod(buffer.ptr, null) == a) break;
    }

    // inf и nan точку не получают
    size_t length = strlen(buffer.ptr);
    if (strpbrk(buffer.ptr, ".eEn") is null)
    {
        buffer[length++] = '.';
        buffer[length++] = '0';
        buffer[length] = 0;
    }

    char* result = cast(char*)GC_malloc_atomic(length + 1);
    strcpy(result, buffer.ptr);
    return result;
}
