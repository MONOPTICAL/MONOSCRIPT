module mono.std.strings.strings;

import mono.std.core.abi : MsArray, newArray;
import mono.std.core.rt : ms_panic, ms_panic_index;

import mono.std.core.abi : GC_malloc_atomic;

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

// Настоящий strlen из libc не перекрываем: символ из этой библиотеки попал бы во все вызовы в процессе
extern(C) int str_len(const char* str) nothrow @nogc
{
    import core.stdc.string : strlen;
    return cast(int) strlen(str);
}

extern(C) char* getIndex(const char* str, int index) nothrow @nogc
{
    import core.stdc.string : strlen;

    const int length = cast(int) strlen(str);
    if (index < 0 || index >= length) ms_panic_index(index, length, 0);

    char* result = cast(char*) GC_malloc_atomic(2);
    result[0] = str[index];
    result[1] = 0;
    return result;
}

extern(C) char* substr(const char* str, int start, int length) nothrow @nogc
{
    import core.stdc.string : memcpy, strlen;

    const int total = cast(int) strlen(str);
    if (start < 0 || length < 0 || start > total || length > total - start)
        ms_panic("substr: range is outside the string", 0);

    char* result = cast(char*) GC_malloc_atomic(length + 1);
    memcpy(result, str + start, length);
    result[length] = 0;
    return result;
}

extern(C) int toInt(const char* str) nothrow @nogc
{
    import core.stdc.stdlib : strtol;
    return cast(int) strtol(str, null, 10);
}

extern(C) int strfind(const char* str, const char* substr) nothrow @nogc
{
    import core.stdc.string : strstr;
    const char* pos = strstr(str, substr);
    if (pos is null) return -1;
    return cast(int)(pos - str);
}

// Пустые части сохраняются: "a,,b" это три строки, как в Python
extern(C) MsArray* split(const char* str, const char* delim) nothrow @nogc
{
    import core.stdc.string : memcpy, strlen, strstr;

    const size_t delimLength = strlen(delim);
    if (delimLength == 0) ms_panic("split: empty delimiter", 0);

    int count = 1;
    for (const(char)* found = strstr(str, delim); found !is null; found = strstr(found + delimLength, delim))
        count++;

    auto result = newArray((char*).sizeof, count, true);
    auto parts = cast(char**) result.data;

    const(char)* start = str;
    for (int i = 0; i < count; i++)
    {
        const(char)* end = strstr(start, delim);
        const size_t length = end !is null ? end - start : strlen(start);

        parts[i] = cast(char*) GC_malloc_atomic(length + 1);
        memcpy(parts[i], start, length);
        parts[i][length] = 0;

        start += length + delimLength;
    }
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
    import core.stdc.stdio : snprintf;
    import core.stdc.stdlib : strtof;
    import core.stdc.string : strlen, strcpy, strpbrk;

    // Самая короткая запись, которая читается обратно в то же число, как у toString_f64
    char[40] buffer;
    foreach (precision; 6 .. 10)
    {
        snprintf(buffer.ptr, buffer.length, "%.*g", precision, cast(double) a);
        if (strtof(buffer.ptr, null) == a) break;
    }

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
