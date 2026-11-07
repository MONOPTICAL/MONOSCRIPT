module mono.std.strings.strings;

import mono.std.core.abi : MsArray, newArray;
import mono.std.core.rt : ms_panic;

import mono.std.core.abi : GC_malloc_atomic;

// Склейка и поиск

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

extern(C) int strfind(const char* str, const char* substr) nothrow @nogc
{
    import core.stdc.string : strstr;
    const char* pos = strstr(str, substr);
    if (pos is null) return -1;
    return cast(int)(pos - str);
}

extern(C) bool startsWith(const char* str, const char* prefix) nothrow @nogc
{
    import core.stdc.string : strncmp, strlen;
    const size_t prefixLength = strlen(prefix);
    return strncmp(str, prefix, prefixLength) == 0;
}


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

extern(C) int count(const char* str, const char* delim) nothrow @nogc
{
    import core.stdc.string : strlen;

    const size_t needle = strlen(delim);
   
    if (needle == 0)
        return cast(int) strlen(str) + 1;

    int found = 0;
    for (size_t i = 0; str[i] != 0; )
    {
        if (strfind(str + i, delim) == 0)
        {
            found++;
            i += needle; // без перекрытия, как str.count в Python
        }
        else
            i++;
    }
    return found;
}

// Строка и массив символов

extern(C) MsArray* toArray(const char* str) nothrow @nogc
{
    import core.stdc.string : memcpy, strlen;
    const int length = cast(int) strlen(str);
    auto array = newArray(1, length, false);
    memcpy(array.data, str, length);
    return array;
}

extern(C) char* joinChars(MsArray* array) nothrow @nogc
{
    import core.stdc.string : memcpy;

    // data не C-строка: GC_malloc_atomic не ставит '\0', длина только в заголовке
    char* result = cast(char*) GC_malloc_atomic(array.length + 1);
    if (array.length > 0)
        memcpy(result, array.data, array.length);
    result[array.length] = 0;
    return result;
}

// Разбор

extern(C) int toInt(const char* str) nothrow @nogc
{
    import core.stdc.stdlib : strtol;
    return cast(int) strtol(str, null, 10);
}

// В строку

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


extern(C) char* toString_char(const char c) nothrow @nogc
{
    char* result = cast(char*) GC_malloc_atomic(2);
    result[0] = c;
    result[1] = 0;
    return result;
}
