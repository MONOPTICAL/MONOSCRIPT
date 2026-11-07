module mono.std.core.abi;

// Как объекты языка лежат в памяти. Раскладка совпадает с кодгеном (CodeGenContext::getArrayStruct):
// функция на D, которая принимает или возвращает array<T>, работает с этим заголовком
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
    array.data = hasReferences ? GC_malloc(bytes) : GC_malloc_atomic(bytes);
    array.length = length;
    array.capacity = length;
    return array;
}
