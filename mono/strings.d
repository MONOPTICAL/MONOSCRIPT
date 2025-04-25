module mono.strings;

extern(C) char* concat(const char* a, const char* b) nothrow @nogc
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