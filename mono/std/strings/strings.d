module mono.std.strings.strings;

import mono.std.core.abi : MsArray, newArray, stringFromCString;
import mono.std.core.rt : ms_raise, CODE;
import mono.std.core.abi : MsPanicContext;

import mono.std.core.abi : newString, stringFromBytes, MsResult, resultOk, resultErr;

// Склейка и поиск

extern(C) char* scat(const char* a, const char* b, MsPanicContext* panic) nothrow @nogc
{
    import core.stdc.string : memcpy;
    import mono.std.core.rt : ms_str_len;

    // Берём из метаданных длину строки
    // {i64 len, [len x i8] data}
    //           a здесь указывает на data внутри структуры, а не на начало структуры

    // потому что мы получаем ссылку на data то нам надо передвинуть указатель на 8 байт назад
    size_t len_a = ms_str_len(a);
    size_t len_b = ms_str_len(b);

    char* result = newString(len_a + len_b, panic);
    if (result is null) return null;

    memcpy(result, a, len_a);
    memcpy(result + len_a, b, len_b);

    return result;
}

extern(C) char* substr(const char* str, int start, int length, MsPanicContext* panic) nothrow @nogc
{
    import core.stdc.string : memcpy, strlen;

    const int total = cast(int) strlen(str);
    if (start < 0 || length < 0 || start > total || length > total - start)
        { ms_raise("substr: range is outside the string", 0, panic); return null; }

    char* result = newString(length, panic);
    if (result is null) return null;
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

extern(C) bool endsWith(const char* str, const char* suffix) nothrow @nogc
{
    import core.stdc.string : strncmp, strlen;
    const size_t strLength = strlen(str);
    const size_t suffixLength = strlen(suffix);
    if (suffixLength > strLength)
        return false;
    return strncmp(str + strLength - suffixLength, suffix, suffixLength) == 0;
}


extern(C) MsArray* split(const char* str, const char* delim, MsPanicContext* panic) nothrow @nogc
{
    import core.stdc.string : memcpy, strlen, strstr;

    const size_t delimLength = strlen(delim);
    if (delimLength == 0) { ms_raise("split: empty delimiter", 0, panic); return null; }

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

        parts[i] = newString(length, panic);
        if (parts[i] is null) return null;
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

// Разбор

// Strict decimal parsing: optional leading sign, no whitespace or trailing junk.
extern(C) MsResult!int* toInt(const char* str) nothrow @nogc
{
    size_t i = 0;
    bool negative = str[0] == '-';
    if (negative || str[0] == '+') i++;
    if (str[i] == 0) return resultErr!int(CODE.INVALID, "expected a decimal integer");
    uint value = 0;
    const uint limit = negative ? 2147483648u : 2147483647u;
    for (; str[i] != 0; i++)
    {
        if (str[i] < '0' || str[i] > '9')
            return resultErr!int(CODE.INVALID, "expected a decimal integer");
        uint digit = str[i] - '0';
        if (value > (limit - digit) / 10)
            return resultErr!int(CODE.OUT_OF_RANGE, "integer is outside i32 range");
        value = value * 10 + digit;
    }
    return resultOk!int(negative ? cast(int)(0u - value) : cast(int) value);
}

extern(C) char* reverse(const char* str, MsPanicContext* panic) nothrow @nogc
{
    import core.stdc.string : strlen, memcpy;

    size_t length = strlen(str);
    char* result = newString(length, panic);
    if (result is null) return null;
    for (size_t i = 0; i < length; i++)
        result[i] = str[length - 1 - i];
    result[length] = 0;
    return result;
}

extern(C) char* lower(const char* str, MsPanicContext* panic) nothrow @nogc
{
    import core.stdc.string : strlen;

    size_t length = strlen(str);
    char* result = newString(length, panic);
    if (result is null) return null;
    for (size_t i = 0; i < length; i++)
    {
        char c = str[i];
        if (c >= 'A' && c <= 'Z')
            c += 'a' - 'A';
        result[i] = c;
    }
    result[length] = 0;
    return result;
}

extern(C) char* upper(const char* str, MsPanicContext* panic) nothrow @nogc
{
    import core.stdc.string : strlen;

    size_t length = strlen(str);
    char* result = newString(length, panic);
    if (result is null) return null;
    for (size_t i = 0; i < length; i++)
    {
        char c = str[i];
        if (c >= 'a' && c <= 'z')
            c -= 'a' - 'A';
        result[i] = c;
    }
    result[length] = 0;
    return result;
}

extern(C) char* strim(const char* str, MsPanicContext* panic) nothrow @nogc
{
    import core.stdc.string : strlen;

    if (str is null) return null;
    size_t start = 0;
    size_t end = strlen(str);
    while (start < end && (str[start] == ' ' || str[start] == '\t' || str[start] == '\n' || str[start] == '\r'))
        start++;
    while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t' || str[end - 1] == '\n' || str[end - 1] == '\r'))
        end--;
    size_t length = end - start;
    char* result = newString(length, panic);
    if (result is null) return null;
    for (size_t i = 0; i < length; i++)
        result[i] = str[start + i];
    result[length] = 0;
    return result;
}

// Проверка

extern(C) bool isNumeric(const char* str, MsPanicContext* panic) nothrow @nogc
{
    if (str is null) return false;
    size_t i = 0;
    if (str[0] == '-' || str[0] == '+') i++;
    if (str[i] == 0) return false;
    for (; str[i] != 0; i++)
    {
        if (str[i] < '0' || str[i] > '9')
            return false;
    }
    return true;
}

extern(C) bool isSpace(const char* str, MsPanicContext* panic) nothrow @nogc
{
    if (str is null) return false;
    for (size_t i = 0; str[i] != 0; i++)
    {
        char c = str[i];
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
            return false;
    }
    return true;
}

// В строку

extern(C) char* toString_int(const int a, MsPanicContext* panic) nothrow @nogc
{
    import core.stdc.stdio : sprintf;
    import core.stdc.string : strlen, strcpy;

    char[20] buffer;
    sprintf(buffer.ptr, "%d", a);

    size_t len_a = strlen(buffer.ptr);

    char* result = newString(len_a, panic);
    if (result is null) return null;

    strcpy(result, buffer.ptr);

    return result;
}

extern(C) char* toString_long(const long a, MsPanicContext* panic) nothrow @nogc
{
    import core.stdc.stdio : sprintf;
    import core.stdc.string : strlen, strcpy;

    char[32] buffer;
    sprintf(buffer.ptr, "%ld", a);

    size_t len_a = strlen(buffer.ptr);

    char* result = newString(len_a, panic);
    if (result is null) return null;

    strcpy(result, buffer.ptr);

    return result;
}

extern(C) char* toString_ulong(const ulong a, MsPanicContext* panic) nothrow @nogc
{
    import core.stdc.stdio : sprintf;
    import core.stdc.string : strlen, strcpy;

    char[32] buffer;
    sprintf(buffer.ptr, "%lu", a);

    char* result = newString(strlen(buffer.ptr), panic);
    if (result is null) return null;
    strcpy(result, buffer.ptr);
    return result;
}

extern(C) char* toString_bool(const bool a, MsPanicContext* panic) nothrow @nogc
{
    import core.stdc.string : strlen, strcpy;

    const char* str = a ? "true" : "false";
    size_t len_a = strlen(str);

    char* result = newString(len_a, panic);
    if (result is null) return null;

    strcpy(result, str);

    return result;
}

extern(C) char* toString_float(const float a, MsPanicContext* panic) nothrow @nogc
{
    import core.stdc.stdio : snprintf;
    import core.stdc.stdlib : strtof;
    import core.stdc.string : strlen, strcpy, strpbrk;

    if (a != a) return stringFromCString("nan", panic);

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

    char* result = newString(length, panic);
    if (result is null) return null;
    strcpy(result, buffer.ptr);
    return result;
}

// Самая короткая запись, которая читается обратно в то же число: 0.1, а не 0.100000 и не 0.10000000000000001.
// Целое значение пишем с .0, чтобы было видно, что это не i64
extern(C) char* toString_f64(const double a, MsPanicContext* panic) nothrow @nogc
{
    import core.stdc.stdio : snprintf;
    import core.stdc.stdlib : strtod;
    import core.stdc.string : strlen, strcpy, strpbrk;

    if (a != a) return stringFromCString("nan", panic);

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

    char* result = newString(length, panic);
    if (result is null) return null;
    strcpy(result, buffer.ptr);
    return result;
}


extern(C) char* toString_char(const char c, MsPanicContext* panic) nothrow @nogc
{
    if (c == 0) { ms_raise("strings cannot contain a NUL byte", 0, panic); return null; }
    char* result = newString(1, panic);
    if (result is null) return null;
    result[0] = c;
    result[1] = 0;
    return result;
}

extern(C) char* toString_char_array(MsArray* array, MsPanicContext* panic) nothrow @nogc
{
    import core.stdc.string : memcpy;

    return stringFromBytes(cast(const char*) array.data, array.length, panic);
}