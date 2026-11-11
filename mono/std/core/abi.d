module mono.std.core.abi;

// Как объекты языка лежат в памяти. Раскладка совпадает с кодгеном (CodeGenContext::getArrayStruct):
// функция на D, которая принимает или возвращает array<T>, работает с этим заголовком
// Explicit per-call panic channel; no process-wide or thread-local error state.
// LLVM layout: { i8 pending, i32 line, ptr message, ptr stackLimit }.
struct MsPanicContext
{
    /** 
     * Вот эта истеричка передаётся в каждую функцию, и она именно та самая которая отвечает за панику
     * pending - Меняеться с помощью ms_raise, он нужен чтобы программа не вылетала к хуям собачим сразу
     *           А только когда паника обрабатываеться
     * line - Номер строки, на которой произошла паника
     * message - Сообщение о панике
     * stackLimit - Ограничение стека, устанавливается один раз через ms_stack_init, сохраняется при пойманных паниках
     *              Проверяется в каждом entry функции, чтобы не выйти за пределы стека
     * entry:
        %0 = getelementptr inbounds nuw { i8, i32, ptr, ptr }, ptr %panic.context, i32 0, i32 3
        %stack.limit = load ptr, ptr %0, align 8
        %sp = call ptr @llvm.stacksave.p0()
        %1 = icmp ult ptr %sp, %stack.limit
        br i1 %1, label %stack.overflow, label %stack.ok
     * Сначала получаем этот самый истеричный контекст из аргументов функции, обычно это MsPanicContext* panic(ptr %panic.context)
     * Потом получаем лимит стека из этого контекста, и сравниваем его с результатом llvm.stacksave, чтобы понять, не вышли ли мы за пределы стека 
     * llvm.stacksave.p0() - сохраняет указатель текущео стека
     * %1 - Сравнивает указатель текущего стека с лимитом стека оператором ult (unsigned less than)
     * Если %1 истинно, значит произошёл stack overflow потому что указатель текущего стека стал меньше лимита стека
     * Стек растёт вниз в сторону меньших адресов
     * 0x9000 - Начало стека
     * 0x8800 - Свободное место
     * 0x8000 - Лимит стека
     */
    bool pending; 
    int line;
    const(char)* message;
    const(void)* stackLimit;
}

struct MsArray
{
    void* data;
    int length;
    int capacity;
}



extern(C) nothrow @nogc
{
    void* GC_malloc(size_t size);
    void* GC_malloc_atomic(size_t size);
}

// Новый массив с готовым местом под элементы.
// hasReferences: в элементах есть ссылки, например строки. Такую память сборщик просматривает, без ссылок нет, и она не обнулена
MsArray* newArray(size_t elementSize, int length, bool hasReferences) nothrow @nogc
{
    auto array = cast(MsArray*) GC_malloc(MsArray.sizeof);
    const size_t bytes = elementSize * length;
    array.data = bytes == 0 ? null
               : hasReferences ? GC_malloc(bytes) : GC_malloc_atomic(bytes);
    array.length = length;
    array.capacity = length;
    return array;
}

// result<T>: функция на D, которая возвращает result<T>, отдаёт указатель на эту структуру.
// Раскладка совпадает с кодгеном (CodeGenContext::getResultStruct). Для result<void> поле value не нужно: используйте MsResultVoid
struct MsResult(T)
{
    bool ok;
    int code;
    const(char)* error;
    T value;
}

struct MsResultVoid
{
    bool ok;
    int code;
    const(char)* error;
}

MsResult!T* resultOk(T)(T value) nothrow @nogc
{
    auto result = cast(MsResult!T*) GC_malloc(MsResult!T.sizeof);
    result.ok = true;
    result.error = stringFromCString("");
    result.value = value;
    return result;
}

MsResult!T* resultErr(T)(int code, const(char)* message) nothrow @nogc
{
    auto result = cast(MsResult!T*) GC_malloc(MsResult!T.sizeof);
    result.ok = false;
    result.code = code;
    result.error = stringFromCString(message);
    return result;
}

MsResultVoid* resultVoidOk() nothrow @nogc
{
    auto result = cast(MsResultVoid*) GC_malloc(MsResultVoid.sizeof);
    result.ok = true;
    result.error = stringFromCString("");
    return result;
}

MsResultVoid* resultVoidErr(int code, const(char)* message) nothrow @nogc
{
    auto result = cast(MsResultVoid*) GC_malloc(MsResultVoid.sizeof);
    result.ok = false;
    result.code = code;
    result.error = stringFromCString(message);
    return result;
}

// A language string points to bytes following a u64 length. The trailing NUL
// preserves interoperability with libc; embedded NUL bytes are rejected.
char* newString(size_t length, MsPanicContext* panic = null) nothrow @nogc
{
    import mono.std.core.rt : ms_raise;
    if (length > int.max) { ms_raise("string is too long", 0, panic); return null; }
    auto allocation = cast(ulong*) GC_malloc_atomic(ulong.sizeof + length + 1);
    *allocation = length;
    auto data = cast(char*) (allocation + 1);
    data[length] = 0;
    return data;
}

char* stringFromBytes(const char* bytes, size_t length, MsPanicContext* panic = null) nothrow @nogc
{
    import core.stdc.string : memcpy, memchr;
    import mono.std.core.rt : ms_raise;
    if (length && memchr(bytes, 0, length) !is null)
        { ms_raise("strings cannot contain a NUL byte", 0, panic); return null; }
    auto data = newString(length, panic);
    if (data is null) return null;
    if (length) memcpy(data, bytes, length);
    return data;
}

char* stringFromCString(const char* text, MsPanicContext* panic = null) nothrow @nogc
{
    import core.stdc.string : strlen;
    return stringFromBytes(text, strlen(text), panic);
}
