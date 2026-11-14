# Кодген MONOSCRIPT: подробная экскурсия

> Ветка `dev`, коммит `c0c9858`. LLVM 18.
> Все IR-примеры в документе **настоящие**: я собрал `ms` (Debug и Release), прогнал примеры и снял IR **до** O2 (то, что печатает Debug-сборка между `--- LLVM IR ---` и `--- END LLVM IR ---`). Ничего не придумано.
> Кода для переделки тут нет. Есть цитаты из твоего кода с разбором по строкам, схемы и примеры. Переписываешь сам 😏

---

## Оглавление

- [Часть 0. Как читать этот документ](#часть-0-как-читать-этот-документ)
- [Часть 1. Архитектура в картинках](#часть-1-архитектура-в-картинках)
- [Часть 2. Состояние: что лежит в `CodeGenContext`](#часть-2-состояние-что-лежит-в-codegencontext)
- [Часть 3. Фундамент: типы, строки, массивы](#часть-3-фундамент-типы-строки-массивы)
- [Часть 4. Паники: соглашение о вызовах построчно](#часть-4-паники-соглашение-о-вызовах-построчно)
- [Часть 5. Программа и функции](#часть-5-программа-и-функции)
- [Часть 6. Переменные и значения (и проклятый `loadValueIfPointer`)](#часть-6-переменные-и-значения)
- [Часть 7. Управление потоком: if, while, for](#часть-7-управление-потоком-if-while-for)
- [Часть 8. Выражения: операторы и касты](#часть-8-выражения-операторы-и-касты)
- [Часть 9. Вызовы: `visit(CallNode)`](#часть-9-вызовы-visitcallnode)
- [Часть 10. Цепочки доступа: `a[i].f.m()`](#часть-10-цепочки-доступа)
- [Часть 11. Структуры и map](#часть-11-структуры-и-map)
- [Часть 12. `result<T>`, `?` и `attempt`](#часть-12-resultt--и-attempt)
- [Часть 13. Сколько это стоит после O2](#часть-13-сколько-это-стоит-после-o2)
- [Часть 14. Где говно (с проверенными фактами)](#часть-14-где-говно)
- [Часть 15. План переделки](#часть-15-план-переделки)
- [Часть 16. Вопросы, на которые ответить до переделки](#часть-16-вопросы-до-переделки)

---

## Часть 0. Как читать этот документ

- Рядом открой репу. Ссылки вида `CodeGenContext.cpp:125` это файл и строка в `dev`.
- В цитатах кода стоят маркеры `// ①`, `// ②`, а под цитатой объяснение по ним.
- IR в примерах **неоптимизированный**. Имена вроде `%left_load`, `%calltmp` взяты из твоих же `CreateXxx(..., "имя")`, так что по ним легко найти место в коде.
- Пролог каждой функции (проверка стека) после первого показа я вырезаю и пишу `; …пролог…`.

Если хочешь воспроизвести сам:

```bash
cmake -B build-dbg -DCMAKE_BUILD_TYPE=Debug && cmake --build build-dbg --target ms
./ms --run file.ms 2>/dev/null | sed -n '/^--- LLVM IR ---$/,/^--- END LLVM IR ---$/p'
```

---

## Часть 1. Архитектура в картинках

### 1.1. Весь пайплайн

```
 file.ms
    │
    ▼
┌─────────┐   ┌─────────┐   ┌──────────────┐   ┌─────────────────────┐
│  Lexer  │──▶│ Parser  │──▶│    Linker    │──▶│  TypeSymbolVisitor  │  ← семантика
└─────────┘   └─────────┘   │ (импорты,    │   │  • выводит типы      │
                            │  std.*)      │   │  • ставит implicitCastTo
                            └──────────────┘   │  • переписывает op:  │
                                               │    "+" → add/fadd/scat
                                               │  • мономорфизирует   │
                                               │    дженерики         │
                                               └──────────┬──────────┘
                                                          │ AST с типами
                                                          ▼
                                     ┌───────────────────────────────────┐
                                     │  ASTGen  (src/runtime/)           │  ← ТЫ ЗДЕСЬ
                                     │  AST → llvm::Module               │
                                     └───────────────┬───────────────────┘
                                                     │
                         ┌───────────────────────────┴──────────────────────┐
                         ▼                                                  ▼
              ┌─────────────────────┐                          ┌───────────────────────┐
              │ --run  (runner.cpp) │                          │ --compile (compile.cpp)│
              │ dlopen libm_std.so  │                          │ O2 → .ll → llc → .o    │
              │ O2 → ORC LLJIT      │                          │ gcc .o libm_std.so     │
              │ вызвать main()      │                          │     -lgc -lm -ldl      │
              └─────────────────────┘                          └───────────────────────┘
                         ▲                                                  ▲
                         └──────────── mono/std/core/*.d (рантайм на D) ────┘
                                       ms_raise, ms_panic, ms_array_reserve,
                                       ms_map_*, ms_str_*, ms_stack_init …
```

Главная мысль: **кодген ничего не проверяет**, он верит семантике. Типы уже выведены (`inferredType`), касты уже решены (`implicitCastTo`), операторы уже переименованы (`op == "icmp_slt"`, а не `"<"`). Кодген только переводит это в IR.

### 1.2. Файлы и кто кого зовёт

```
                    ┌────────────────────────────────────────┐
                    │ ASTGen (ASTGen.cpp, 1161 строк)        │
                    │  visit(ProgramNode)                    │
                    │  visit(FunctionNode)  visit(If/For/…)  │
                    │  visit(CallNode) ← 200 строк лесенки   │
                    └──┬──────────┬──────────┬───────────┬───┘
                       │          │          │           │
          ┌────────────▼─┐  ┌─────▼──────┐ ┌─▼────────┐ ┌▼───────────────┐
          │ Declarations │  │ Expressions│ │ Arrays   │ │ Objects / Maps │
          │ let / global │  │ + - * / ?  │ │ литералы │ │ struct, map    │
          │ reassign     │  │ try        │ │ a[i].f   │ │                │
          └──────┬───────┘  └─────┬──────┘ └────┬─────┘ └──────┬─────────┘
                 │                │             │              │
                 └───────┬────────┴──────┬──────┴──────────────┘
                         ▼               ▼
              ┌────────────────────┐  ┌──────────────────────────────────┐
              │ TypeConversions    │  │ CodeGenContext                   │
              │ loadValueIfPointer │  │  Builder, Module, NamedValues    │
              │ convertValueToType │  │  emitCall / checkPanic / …       │
              │ applyImplicitCast  │  │  createArray / getArrayElementPtr│
              │ applyCharCast      │  │  createString / getLLVMType      │
              └────────────────────┘  └──────────────────────────────────┘
```

Обрати внимание на одну неочевидную вещь: хелперы (`Declarations`, `Arrays`, `Statements`…) **сами создают новый `ASTGen`** через `ASTGen codeGen(context);` и через него генерируют подвыражения. Визитор не один, их десятки, и все разделяют один `CodeGenContext`. Причина в том, что результат визита лежит в поле `result` (часть 14.3).

### 1.3. Как выглядит программа на выходе

```
 ┌─────────────────────────────────────────────────────────────┐
 │ define i32 @main()            ← бывший __ms_start           │
 │   alloca PanicContext                                       │
 │   ms_rt_init()                                              │
 │   ms_stack_init(&ctx)                                       │
 │   <инициализация глобалок по порядку>                       │
 │   r = __ms_user_main(&ctx)    ← твой main                   │
 │   if ctx.pending → ms_panic(msg, line)   [фатал, exit 101]  │
 │   ret r                                                     │
 ├─────────────────────────────────────────────────────────────┤
 │ define internal i32 @__ms_user_main(ptr %ctx)               │
 │ define internal <T> @f(args…, ptr %ctx)    ← каждая функция │
 │ define internal <T> @P.method(ptr self, args…, ptr %ctx)    │
 │ define private ptr @__ms_new_P(ptr %ctx)   ← конструктор    │
 │ define private <T> @__ms_adapter_xxx(…, ptr %ctx)           │
 ├─────────────────────────────────────────────────────────────┤
 │ declare … @ms_*, @GC_malloc, @strcmp, stdlib из mono.toml   │
 └─────────────────────────────────────────────────────────────┘
```

---

## Часть 2. Состояние: что лежит в `CodeGenContext`

`headers/CodeGenContext.h`. Это по сути **глобальное состояние кодгена**, и большая часть багов в кодгенах обычно растёт из того, что какое-то поле забыли сохранить и восстановить. Поэтому вот каждое поле:

| Поле | Что это | Кто меняет и восстанавливает |
|---|---|---|
| `TheContext`, `OwnedContext` | LLVMContext. Если дали внешний (JIT), то используется он, иначе свой | конструктор |
| `Builder` | IRBuilder, «курсор», куда вставляются инструкции | все |
| `TheModule` | модуль | конструктор |
| `NamedValues` | `имя → Value*` (alloca или global) | **копируется целиком** на входе в блок и восстанавливается на выходе (if/while/for/attempt/функция) |
| `StartFunction` | `__ms_start`, пока он генерируется. Используется как признак «мы в глобальном контексте» | `visit(ProgramNode)`, обнуляется в `finishStartFunction` |
| `CurrentReturnType` | тип возврата текущей функции (AST-тип). Нужен для `return [литерал]` и для `?` | `visit(FunctionNode)` сохраняет и восстанавливает |
| `Structs` | `имя структуры → StructLayout` | `Objects::registerStruct` |
| `CurrentLine` | строка текущей инструкции, для сообщений паник | `visit(BlockNode)`, `visit(ProgramNode)`, конструктор |
| `loopEndBlocks`, `loopCondBlocks` | стеки для `break`/`continue` | `push/popLoopContext` |
| `ResultHandlers` | стек обработчиков ошибок `result` внутри `attempt` | `visit(AttemptNode)`, **обнуляется** на входе в функцию |
| `PanicHandlers` | стек обработчиков паник внутри `attempt` | то же |
| `PanicContext` | `Value*` указателя на PanicContext **в текущей функции** (последний аргумент, а в `__ms_start` это alloca) | `visit(FunctionNode)`, `emitConstructor` |

`StructLayout`:

```cpp
struct StructLayout {
    StructNode*                 node;          // AST структуры
    llvm::StructType*           type;          // %ms_struct_P = type { … }
    llvm::Function*             constructor;   // @__ms_new_P
    std::map<string, unsigned>  fieldIndex;    // "age" → 0
    vector<VariableAssignNode>  fields;        // поля с их значениями по умолчанию
    map<string, FunctionNode>   methods;       // методы
    bool                        hasReferences; // есть ли поле-указатель → GC_malloc или GC_malloc_atomic
};
```

---

## Часть 3. Фундамент: типы, строки, массивы

### 3.1. `getLLVMType`: AST-тип → LLVM-тип (`CodeGenContext.cpp:22`)

```cpp
if (auto simpleType = std::dynamic_pointer_cast<SimpleTypeNode>(typeNode)) {
    if (simpleType->name == "i1")  return getInt1Ty(ctx);
    ...
    if (simpleType->name == "char")   return getInt8Ty(ctx);       // ①
    if (simpleType->name == "string") return PointerType::get(...); // ②
    if (simpleType->name == "void")   return getVoidTy(ctx);
    if (simpleType->name == "null")   return ptr;
    if (simpleType->name == "auto")   return ptr;                  // ③
}
else if (auto genericType = ...GenericTypeNode...) {
    return PointerType::getUnqual(ctx);                            // ④
}
return PointerType::getUnqual(ctx);                                // ⑤
```

1. `char` это `i8`. Беззнаковость **не выражена в типе**, она обеспечивается `zext` в `applyCharCast` (часть 8.4).
2. Строка это просто `ptr`.
3. `auto` до кодгена доходить не должен (семантика выводит тип), но если дойдёт, то тихо станет `ptr`.
4. Любой дженерик (`array<…>`, `map<…>`, `result<…>`, `func<…>`) это `ptr`. Коллекции живут в куче, переменная хранит ссылку.
5. **Всё остальное** (имя структуры `P`, `panic_info`, опечатка) это тоже `ptr`. Тихий фолбэк: если семантика пропустит неизвестный тип, кодген не заметит. TODO в коде про `array<array<…>>` уже неактуален, вложенные массивы работают, потому что всё равно `ptr`.

### 3.2. `createString`: строковые литералы (`CodeGenContext.cpp:252`)

```cpp
if (text.find('\0') != npos) throw ...;                         // ①
auto bytes = ConstantDataArray::getString(TheContext, text, true); // ②
auto value = ConstantStruct::getAnon({getInt64(text.size()), bytes}); // ③
auto global = new GlobalVariable(*TheModule, value->getType(), true,
                                 PrivateLinkage, value, name);   // ④
global->setAlignment(Align(8));
Constant* indices[] = {getInt32(0), getInt32(1), getInt32(0)};
return ConstantExpr::getInBoundsGetElementPtr(value->getType(), global, indices); // ⑤
```

1. NUL в строке запрещён, иначе сломаются `strcmp` и `for c in s`, которые идут до нуля.
2. Байты **с** завершающим `\0` (`true`).
3. Анонимная структура `{ i64 длина, [N x i8] байты }`.
4. Константная приватная глобалка.
5. Возвращается указатель **на байты**, а не на структуру. Поэтому строку можно отдать в C-функцию как `char*`, а длина лежит **за 8 байт до** указателя, и рантайм её оттуда читает.

```
            ┌──────────────┬───┬───┬───┬───┬────┐
 @str  ───▶ │ i64 len = 4  │ t │ e │ x │ t │ \0 │
            └──────────────┴───┴───┴───┴───┴────┘
                           ▲
                           └── вот это значение строки (ptr)
```

Реальный IR (из любого примера):

```llvm
@str = private constant { i64, [15 x i8] } { i64 14, [15 x i8] c"stack overflow\00" }, align 8
; использование:
ptr getelementptr inbounds ({ i64, [15 x i8] }, ptr @str, i32 0, i32 1, i32 0)
```

⚠️ **Дедупликации нет.** Каждый вызов создаёт новую глобалку. Строка `"stack overflow"` вставляется в пролог **каждой** функции, поэтому в программе из 9 функций её 9 копий (проверил на `func_value_void_and_types.ms`: 9 штук). O2 их **не** склеивает, я проверил на `01_add`: после O2 остались и `@str`, и `@str.1`. Кэш `текст → Constant*` решает это за 5 строк.

### 3.3. Массивы: `createArray`, `getArrayElementPtr` (`CodeGenContext.cpp:95`, `:125`)

Раскладка:

```
 переменная a ──▶ ┌──────────────────────────┐   GC_malloc (есть ссылка data)
                  │ %ms_array                │
                  │  ptr data ───────────────┼──▶ ┌────┬────┬────┬────┐  GC_malloc_atomic, если
                  │  i32 length = 2          │    │ 10 │ 20 │    │    │  элементы не указатели,
                  │  i32 capacity = 2        │    └────┴────┴────┴────┘  иначе GC_malloc
                  └──────────────────────────┘
```

```cpp
llvm::Value* CodeGenContext::createArray(llvm::Type* elementType, llvm::Value* size) {
    llvm::StructType* arrayStruct = getArrayStruct();                 // ①
    llvm::Value* header = allocateObject(getInt64(sizeof(ms_array)), true, "array"); // ②
    llvm::Value* bytes = CreateMul(CreateZExt(size, i64), getInt64(sizeof(elem)));
    llvm::Value* data = allocateObject(bytes, elementType->isPointerTy(), "array_data"); // ③
    CreateStore(data, GEP(header, 0));
    CreateStore(size, GEP(header, 1));                                // ④
    CreateStore(size, GEP(header, 2));
    return header;
}
```

1. `%ms_array` создаётся один раз (`getTypeByName`) и потом переиспользуется.
2. Заголовок всегда `GC_malloc` (не atomic), потому что внутри лежит указатель `data`, и GC должен по нему пройти.
3. Данные atomic, только если элементы не указатели. `array<i32>` → atomic, `array<string>` → нет. **Atomic-память не обнуляется**, но тут это не проблема: литерал сразу заполняет все ячейки.
4. `length = capacity = size`. Запаса нет, первый же `push` вызовет перевыделение в `ms_array_reserve`.

```cpp
llvm::Value* CodeGenContext::getArrayElementPtr(array, elementType, index, line) {
    llvm::Value* wideIndex = index->getType()->isIntegerTy(1)
        ? CreateZExt(index, i64)                       // ①
        : CreateSExt(index, i64);                      // ②
    llvm::Value* length = getArrayLength(array);
    ...
    CreateCondBr(CreateICmpUGE(wideIndex, CreateZExt(length, i64)), failBlock, okBlock); // ③

    SetInsertPoint(failBlock);
    CreateCall(ms_panic_index, {wideIndex, length, line, PanicContext}); // ④
    propagatePanic();                                                    // ⑤

    SetInsertPoint(okBlock);
    return CreateGEP(elementType, getArrayData(array), wideIndex);       // ⑥
}
```

1. `a[true]` → индекс 1, а не -1.
2. Знаковое расширение: `-1` станет `0xFFFF…FFFF`…
3. …и **одна** беззнаковая проверка `>= length` ловит и выход за верх, и отрицательные индексы. Классический трюк, хорошо сделано.
4. `ms_panic_index` объявлен через `getOrDeclareFunction`, который **по имени** знает, что ему нужен 4-й аргумент `PanicContext` (часть 14.5).
5. Дальше паника летит по обычным правилам (часть 4).
6. Адрес элемента. Вызывающий сам решает, load это или store.

Реальный IR для `i32 x = a[1]` (пример 03):

```llvm
  %load4 = load ptr, ptr %a, align 8                       ; a: адрес переменной → значение (ссылка)
  %10 = getelementptr inbounds %ms_array, ptr %load4, i32 0, i32 1
  %array_len = load i32, ptr %10, align 4                  ; длина
  %11 = zext i32 %array_len to i64
  %12 = icmp uge i64 1, %11                                ; 1 >= len ?
  br i1 %12, label %bounds.fail, label %bounds.ok

bounds.fail:
  call void @ms_panic_index(i64 1, i32 %array_len, i32 4, ptr %panic.context)
  ret i32 0                                                ; ← propagatePanic: «вернуть 0, пусть вызывающий проверит флаг»

bounds.ok:
  %13 = getelementptr inbounds %ms_array, ptr %load4, i32 0, i32 0
  %array_data5 = load ptr, ptr %13, align 8
  %element_ptr6 = getelementptr i32, ptr %array_data5, i64 1
  %element = load i32, ptr %element_ptr6, align 4
  store i32 %element, ptr %x, align 4
```

---

## Часть 4. Паники: соглашение о вызовах построчно

Самая важная часть кодгена. Если её понять, всё остальное читается легко.

### 4.1. Идея

C++-исключений и unwind'а нет. Вместо этого **каждая функция языка получает скрытый последний аргумент** `ptr %panic.context`. Это указатель на одну структуру на всю программу (она лежит в стеке `main`):

```
 PanicContext = { i8 pending, i32 line, ptr message, ptr stackLimit }
                    [0]         [1]        [2]          [3]
```

Кто паникует (рантайм или кодген), ставит `pending = 1`, `message`, `line` и **просто возвращается**. А код после каждого вызова проверяет `pending`.

```
   f() вызывает g() вызывает h()
                              │
                              ├─ ms_raise("division by zero")  → ctx.pending = 1
                              └─ ret 0          ← h возвращает мусор (0/null)
                   ┌──────────┘
                   ├─ checkPanic: ctx.pending != 0 ?  → да
                   └─ ret 0          ← g тоже выходит
        ┌──────────┘
        ├─ checkPanic → да → ret 0
 main ◀─┘
   └─ checkPanic → да → ms_panic(message, line)   ← печать и exit(101)
```

Если по дороге есть `attempt`, то вместо `ret` будет `br` в его обработчик.

### 4.2. `emitCall` (`CodeGenContext.cpp:198`)

```cpp
llvm::Value* CodeGenContext::emitCall(llvm::Function* function, vector<Value*> arguments, const string& name) {
    const bool checked = function->hasFnAttribute("ms.panic_context"); // ①
    if (checked) arguments.push_back(PanicContext);                   // ②
    auto value = Builder.CreateCall(function, arguments,
                     function->getReturnType()->isVoidTy() ? "" : name); // ③
    if (checked) checkPanic();                                        // ④
    return value;
}
```

1. Признак «функция языка» это строковый атрибут `"ms.panic_context"`. Его ставят: `declareUserFunction` (все пользовательские), `registerStruct` (конструкторы), `functionAdapter`, TOML-функции с `panic_context = true` и захардкоженный список в `getOrDeclareFunction`.
2. Скрытый аргумент добавляется **сам**, вызывающему думать не надо.
3. У void-вызова нельзя давать имя (LLVM ругнётся), отсюда тернарник.
4. После вызова сразу проверка флага.

**Правило:** всё, что может паниковать, надо вызывать через `emitCall`. Прямой `Builder.CreateCall` в обход него означает потерянную панику. Мест, где это сделано руками, несколько: `visit(CallNode)` для `func<>`-значений вручную добавляет `PanicContext` и зовёт `checkPanic` (так и должно быть, там нет `llvm::Function*`), а `getArrayElementPtr`, `emitPanic` и `ms_panic_result` передают контекст руками.

### 4.3. `checkPanic` (`CodeGenContext.cpp:183`)

```cpp
void CodeGenContext::checkPanic() {
    auto fail = BasicBlock::Create(TheContext, "panic.propagate", function);
    auto ok   = BasicBlock::Create(TheContext, "panic.clear", function);
    auto flag = CreateLoad(i8, GEP(PanicContext, 0));                  // ①
    CreateCondBr(CreateICmpNE(flag, getInt8(0)), fail, ok);
    SetInsertPoint(fail);
    auto lineSlot = GEP(PanicContext, 1);
    auto line = CreateLoad(i32, lineSlot);
    CreateStore(CreateSelect(CreateICmpEQ(line, 0),
                getInt32(displayLine(CurrentLine)), line), lineSlot);  // ②
    propagatePanic();                                                  // ③
    SetInsertPoint(ok);                                                // ④
}
```

1. Читаем `pending`.
2. Если строка паники ещё не известна (`line == 0`, так бывает у нативных функций stdlib, которые строку не знают), подставляем строку **текущего вызова**. Поэтому паника внутри `split()` покажет строку, где ты вызвал `split`.
3. Дальше по правилам из 4.4.
4. Курсор остаётся в «чистой» ветке, и генерация продолжается как ни в чём не бывало.

IR (пример 01, `i32 x = add(2, 3)` в `main`):

```llvm
  %calltmp = call i32 @add(i32 2, i32 3, ptr %panic.context)
  %2 = getelementptr inbounds { i8, i32, ptr, ptr }, ptr %panic.context, i32 0, i32 0
  %3 = load i8, ptr %2, align 1
  %4 = icmp ne i8 %3, 0
  br i1 %4, label %panic.propagate, label %panic.clear

panic.propagate:
  %5 = getelementptr inbounds { i8, i32, ptr, ptr }, ptr %panic.context, i32 0, i32 1
  %6 = load i32, ptr %5, align 4
  %7 = icmp eq i32 %6, 0
  %8 = select i1 %7, i32 5, i32 %6        ; 5 = строка вызова (displayLine)
  store i32 %8, ptr %5, align 4
  ret i32 0

panic.clear:
  store i32 %calltmp, ptr %x, align 4
```

### 4.4. `propagatePanic` (`CodeGenContext.cpp:162`): куда лететь

```cpp
void CodeGenContext::propagatePanic() {
    if (!PanicHandlers.empty()) {                       // ①
        Builder.CreateBr(PanicHandlers.back());
        return;
    }
    auto function = Builder.GetInsertBlock()->getParent();
    if (function->hasFnAttribute("ms.panic_context")) { // ②
        if (function->getReturnType()->isVoidTy()) Builder.CreateRetVoid();
        else Builder.CreateRet(Constant::getNullValue(function->getReturnType()));
        return;
    }
    // ③ мы в __ms_start: дальше бежать некуда
    auto message = CreateLoad(ptr, GEP(PanicContext, 2));
    auto line    = CreateLoad(i32, GEP(PanicContext, 1));
    auto fatal = getOrDeclareFunction("ms_panic", void(ptr, i32));
    fatal->setDoesNotReturn();
    CreateCall(fatal, {message, line});
    CreateUnreachable();
}
```

```
                      propagatePanic()
                            │
            ┌───────────────┼───────────────────────────┐
            ▼               ▼                           ▼
   есть attempt?     функция языка?               это __ms_start
   br %attempt.panic  ret 0 / null / void          ms_panic(msg, line)
                      (вызывающий проверит)         unreachable
```

1. Внутри `attempt … panic x` прыгаем в обработчик.
2. Иначе выходим из функции с нулевым значением. Значение никто не прочитает, потому что вызывающий сначала проверит флаг.
3. На самом верху вызываем фатал.

**Важный инвариант:** после `propagatePanic()` текущий блок **завершён** (`br`/`ret`/`unreachable`). Все вызывающие сразу делают `SetInsertPoint` на новый блок. Если забыть, получишь инструкции после терминатора, и это невалидный IR.

### 4.5. `emitPanic`: паника из кодгена (`CodeGenContext.cpp:150`)

```cpp
void CodeGenContext::emitPanic(const std::string& message, int line) {
    auto raise = getOrDeclareFunction("ms_raise", void(ptr, i32, ptr));
    Builder.CreateCall(raise, {createString(message), getInt32(displayLine(line)), PanicContext});
    propagatePanic();
}
```

Вызывается для: деления на ноль, `MIN / -1`, `pop` пустого массива, float → int вне диапазона, изменения массива во время `for`, переполнения стека. Каждый вызов создаёт **новую** строку (см. 3.2).

### 4.6. Проверка стека в прологе (`ASTGen.cpp`, в `visit(FunctionNode)`)

```cpp
auto limit = CreateLoad(ptr, GEP(PanicContext, 3), "stack.limit");   // ①
...
CreateCondBr(CreateICmpULT(CreateStackSave("sp"), limit), overflow, fine); // ②
SetInsertPoint(overflow);
emitPanic("stack overflow", node.line);
SetInsertPoint(fine);
```

1. `stackLimit` ставит `ms_stack_init` в D: низ стека потока + запас `STACK_RESERVE`.
2. Стек растёт вниз: если указатель стека ниже лимита, то паника. Так бесконечная рекурсия становится обычной ловимой паникой, а не segfault. Красиво.

IR (это и есть «пролог», который дальше я буду вырезать):

```llvm
define internal i32 @add(i32 %a, i32 %b, ptr %panic.context) #0 {
entry:
  %b2 = alloca i32, align 4
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4                ; параметры копируются в alloca
  store i32 %b, ptr %b2, align 4
  %0 = getelementptr inbounds { i8, i32, ptr, ptr }, ptr %panic.context, i32 0, i32 3
  %stack.limit = load ptr, ptr %0, align 8
  %sp = call ptr @llvm.stacksave.p0()
  %1 = icmp ult ptr %sp, %stack.limit
  br i1 %1, label %stack.overflow, label %stack.ok

stack.overflow:
  call void @ms_raise(ptr getelementptr inbounds ({ i64, [15 x i8] }, ptr @str, i32 0, i32 1, i32 0), i32 1, ptr %panic.context)
  ret i32 0

stack.ok:
  %left_load = load i32, ptr %a1, align 4
  %right_load = load i32, ptr %b2, align 4
  %addtmp_int = add i32 %left_load, %right_load
  ret i32 %addtmp_int
}
attributes #0 = { "ms.panic_context" }
```

### 4.7. `functionAdapter`: функция как значение (`CodeGenContext.cpp:207`)

Проблема: `func<>`-значение вызывается **всегда** со скрытым аргументом. Но нативная функция stdlib (например, без `panic_context = true`) его не принимает. Решение: обёртка.

```cpp
llvm::Function* CodeGenContext::functionAdapter(llvm::Function* function) {
    if (function->hasFnAttribute("ms.panic_context")) return function;     // ①
    const std::string name = "__ms_adapter_" + function->getName().str();
    if (auto existing = TheModule->getFunction(name)) return existing;     // ②
    // те же параметры + ptr
    auto adapter = Function::Create(..., PrivateLinkage, name, TheModule.get());
    adapter->addFnAttr("ms.panic_context");
    IRBuilder<> builder(BasicBlock::Create(TheContext, "entry", adapter)); // ③
    // call function(args без последнего), ret результат
}
```

1. Функция языка уже подходит как есть.
2. Одна обёртка на функцию.
3. **Отдельный** `IRBuilder`, чтобы не сбить курсор основного `Builder`. Правильно, запомни этот приём.

---

## Часть 5. Программа и функции

### 5.1. `visit(ProgramNode)` (`ASTGen.cpp:99`)

```cpp
for (auto& n : node.body)
    if (auto structure = dynamic_pointer_cast<StructNode>(n))
        Objects::registerStruct(context, *structure);                    // ①

for (auto& [name, layout] : context.Structs)
    for (auto& [methodName, method] : layout.methods)
        declareUserFunction(context, *method);                           // ②

for (auto& n : node.body)
    if (auto function = ...FunctionNode...; function && function->associated.empty()
                                             && function->typeParameters.empty())
        declareUserFunction(context, *function);                         // ③

context.StartFunction = Function::Create(i32(), ExternalLinkage, "__ms_start", …); // ④
startBlock = BasicBlock::Create(…, "entry", StartFunction);
SetInsertPoint(startBlock);
context.PanicContext = createEntryAlloca(getPanicStruct(), "panic.context");    // ⑤
CreateStore(Constant::getNullValue(getPanicStruct()), PanicContext);
CreateCall(ms_rt_init);
CreateCall(ms_stack_init, {PanicContext});
ClearInsertionPoint();                                                   // ⑥

for (auto& n : node.body) {
    if (dynamic_pointer_cast<VariableAssignNode>(n)) {                   // ⑦
        context.CurrentLine = n->line;
        SetInsertPoint(startBlock);
        n->accept(*this);
        startBlock = GetInsertBlock();                                   // ⑧
        ClearInsertionPoint();
        continue;
    }
    n->accept(*this);                                                    // ⑨
}
finishStartFunction(context, node, startBlock);                          // ⑩
```

1. Раскладки структур нужны первыми: функция может вернуть `P` или создать `P()` раньше объявления структуры.
2. Методы объявляются (только сигнатуры).
3. Свободные функции объявляются. Шаблоны (`typeParameters` не пусто) пропускаются, а их конкретные копии уже лежат в AST как обычные функции (семантика их туда положила).
4. Временное имя `__ms_start`, переименование будет в конце.
5. Единственный **настоящий** PanicContext на всю программу, в стеке `main`.
6. Курсор убирается: дальше генерируются функции, и они ставят свой.
7. Глобалки генерируются **внутри `__ms_start`**, по порядку. Признак «глобальная» это то, что курсор стоит в `StartFunction` (см. `visit(VariableAssignNode)`).
8. Инициализатор глобалки мог создать блоки (например, проверку деления), поэтому продолжаем с последнего.
9. Функции и структуры.
10. Дописывается вызов entry.

### 5.2. `declareUserFunction` (`ASTGen.cpp:45`)

```cpp
const string symbol = node.associated.empty() ? node.name
                                              : node.associated + "." + node.name; // ①
if (auto existing = TheModule->getFunction(symbol)) return existing;               // ②
for (auto& param : node.parameters) paramTypes.push_back(getLLVMType(param.first));
bool isMain = node.associated.empty() && (node.name == "main" || has "@entry");
llvm::Type* returnLLVMType = node.returnType ? getLLVMType(node.returnType) : void;
if (isMain) returnLLVMType = i32;                                                   // ③
paramTypes.push_back(ptr);                                                          // ④
auto function = Function::Create(funcType, InternalLinkage, symbol, TheModule.get()); // ⑤
function->addFnAttr("ms.panic_context");
```

1. Метод `get` структуры `P` называется `P.get`. Точка в имени LLVM-символа разрешена, и коллизий нет.
2. Идемпотентно: можно звать сколько угодно раз.
3. Entry всегда `i32`, даже если в исходнике `[void]main()` (тест `entry_void_main.ms`).
4. Скрытый параметр.
5. `InternalLinkage`, то есть наружу не торчит, и O2 может инлайнить или выкидывать.

У методов `self` это **обычный первый параметр** из `node.parameters` (семантика его туда добавила), поэтому `P.birthday(ptr %self, ptr %panic.context)`.

### 5.3. `finishStartFunction` (`ASTGen.cpp:75`)

```cpp
llvm::Function* entry = TheModule->getFunction(findEntryName(node));
if (!entry) { start->eraseFromParent(); return; }                     // ①
SetInsertPoint(startBlock);
for (unsigned i = 0; i + 1 < entry->arg_size(); ++i)
    arguments.push_back(Constant::getNullValue(entry->getArg(i)->getType())); // ②
CreateRet(context.emitCall(entry, arguments));                        // ③
if (auto userMain = TheModule->getFunction("main")) userMain->setName("__ms_user_main"); // ④
start->setName("main");
```

1. Нет entry, значит нечего запускать. Ошибку покажет `compile.cpp`/runner.
2. Если у entry есть параметры, туда передаются нули.
3. `emitCall` сам вставит `checkPanic`. Мы в `__ms_start`, у него нет атрибута, поэтому `propagatePanic` пойдёт по ветке ③ (фатал).
4. Переименование, чтобы `__ms_start` мог занять имя `main`. Это работает и для JIT (`lookup("main")`), и для `gcc`.

Реальный итог (пример 01):

```llvm
define i32 @main() {
entry:
  %panic.context = alloca { i8, i32, ptr, ptr }, align 8
  store { i8, i32, ptr, ptr } zeroinitializer, ptr %panic.context, align 8
  call void @ms_rt_init()
  call void @ms_stack_init(ptr %panic.context)
  %0 = call i32 @__ms_user_main(ptr %panic.context)
  %1 = getelementptr inbounds { i8, i32, ptr, ptr }, ptr %panic.context, i32 0, i32 0
  %2 = load i8, ptr %1, align 1
  %3 = icmp ne i8 %2, 0
  br i1 %3, label %panic.propagate, label %panic.clear

panic.propagate:
  ; … подставить строку, если 0 …
  %9 = load ptr, ptr %8, align 8         ; message
  %11 = load i32, ptr %10, align 4       ; line
  call void @ms_panic(ptr %9, i32 %11)
  unreachable

panic.clear:
  ret i32 %0
}
```

### 5.4. `visit(FunctionNode)` (`ASTGen.cpp:162`)

Разбор по шагам:

```cpp
if (!node.typeParameters.empty()) return;                     // ① шаблон
// сохранить курсор (на случай вложенной функции)
llvm::Function* func = declareUserFunction(context, node);    // ②

auto outerReturnType = context.CurrentReturnType;             // ③ сохранить…
context.CurrentReturnType = node.returnType;
auto outerPanicContext = context.PanicContext;
auto outerResultHandlers = std::move(context.ResultHandlers);
auto outerPanicHandlers  = std::move(context.PanicHandlers);
context.ResultHandlers.clear(); context.PanicHandlers.clear(); // ④
context.PanicContext = func->getArg(func->arg_size() - 1);    // ⑤

entryBlock = BasicBlock::Create(…, "entry", func);
SetInsertPoint(entryBlock);

auto oldNamedValues = context.NamedValues;
std::erase_if(context.NamedValues, [](auto& e){ return !isa<GlobalValue>(e.second); }); // ⑥

for (auto &arg : func->args()) {                              // ⑦
    if (idx == node.parameters.size()) break;
    arg.setName(node.parameters[idx++].second);
    AllocaInst* paramSlot = createEntryAlloca(arg.getType(), arg.getName());
    CreateStore(&arg, paramSlot);
    context.NamedValues[arg.getName()] = paramSlot;
}

{ /* проверка стека, см. 4.6 */ }                            // ⑧

if (node.body) node.body->accept(*this);                      // ⑨

// ⑩ дописать ret, если тело не завершилось
if (!entryBlock->getTerminator()) { … }
if (currentInsertionBlock != entryBlock && !currentInsertionBlock->getTerminator()) {
    if (isMain) ret 0; else if (void) ret void; else unreachable;
}

context.NamedValues = oldNamedValues;                         // ⑪ …восстановить
context.CurrentReturnType = outerReturnType;
context.PanicContext = outerPanicContext;
context.ResultHandlers = std::move(outerResultHandlers);
context.PanicHandlers  = std::move(outerPanicHandlers);
context.Builder.ClearInsertionPoint();
```

1. Шаблоны не генерируются.
2. Объявление уже есть (из `visit(ProgramNode)`), здесь его просто достают.
3. Функции могут быть **вложенными** (локальная функция в теле другой), поэтому всё состояние сохраняется.
4. **Обработчики `attempt` не протекают внутрь функции.** Если вызвать `f()` внутри `attempt`, паника внутри `f` сделает `ret`, а `attempt` поймает её уже **после** вызова через `checkPanic`. Правильная семантика.
5. Свой PanicContext это последний аргумент.
6. Локальная функция не видит alloca внешней (иначе это было бы обращение к чужому стеку). Остаются только глобалки. Замыканий нет.
7. Каждый параметр копируется в alloca. Поэтому `%a1`, `%b2` в IR, и поэтому параметр можно переприсвоить.
8. Пролог.
9. Тело.
10. Две почти одинаковые проверки (см. 14.7). Смысл такой: если последний блок без терминатора, то `main` получает `ret 0`, void получает `ret void`, а иначе ставится `unreachable` (семантика гарантирует return на всех путях).
11. Восстановление.

---

## Часть 6. Переменные и значения

### 6.1. Главная концепция: «что возвращает визит»

Вот это **надо** держать в голове, иначе кодген не читается:

| Узел | Что лежит в `result` |
|---|---|
| `IdentifierNode` (переменная) | **адрес** переменной: `AllocaInst*` или `GlobalVariable*` |
| `NumberNode`, `FloatNumberNode` | константа-**значение** |
| `StringNode` | `ptr` на байты (**значение**) |
| `BinaryOpNode`, `CallNode`, `AccessExpression` | **значение** |
| `IdentifierNode` (функция как значение) | `llvm::Function*` (значение-указатель) |
| `VariableAssignNode` | alloca (никто не использует) |
| `IfNode`, `WhileNode`, `ForNode`, … | `nullptr` |

То есть одно и то же «что-то типа `ptr`» может быть и **адресом** переменной (который надо загрузить), и **значением** (строка, ссылка на массив, функция), которое грузить нельзя. Различает их `loadValueIfPointer`, и различает **по виду инструкции**, а не по смыслу.

### 6.2. `loadValueIfPointer` (`TypeConversions.cpp:47`) построчно

```cpp
if (!value->getType()->isPointerTy()) return value;              // ①

if (AllocaInst* allocaInst = dyn_cast<AllocaInst>(value)) {      // ②
    Type* loadedType = allocaInst->getAllocatedType();
    if (loadedType->isArrayTy() && loadedType->getArrayElementType()->isIntegerTy(8)) {
        return CreateInBoundsGEP(loadedType, allocaInst, {0, 0}); // ③
    }
    return CreateLoad(loadedType, value, loadName);              // ④
}
else if (GlobalVariable* globalVar = dyn_cast<GlobalVariable>(value)) { // ⑤
    Type* valueType = globalVar->getValueType();
    if (valueType->isArrayTy() && ...isIntegerTy(8)) return GEP; // ③
    return CreateLoad(valueType, value, loadName);
}
return value;                                                    // ⑥
```

1. Не указатель (число), значит уже значение.
2. Это alloca, значит это **переменная**, и её надо загрузить.
3. **Мёртвая ветка.** Раньше строки, видимо, хранились как `[N x i8]` в alloca/global. Сейчас `getLLVMType` никогда не возвращает массивный тип, а литералы это `ConstantExpr` (не `GlobalVariable`). Сюда не попасть.
4. Загрузить тип, который лежит в alloca.
5. То же для глобалки.
6. Любой другой указатель (результат `load`, вызова, GEP, `ConstantExpr`-строки, `Function*`) считается **значением**.

Почему это работает: все переменные это alloca/global, а все значения-указатели это **не** alloca/global. Пока этот инвариант держится, всё ок.

Когда сломается:
- если когда-нибудь появится значение, которое **само по себе** является alloca (например, временный объект на стеке, struct by value, или `method_result` из 10.2, который как раз alloca!). В `emitElementPtr` результат метода кладётся в `createEntryAlloca(..., "method_result")`, и это работает только потому, что дальше по цепочке он используется как **адрес** элемента, а не через `loadValueIfPointer`. Хрупко;
- если глобалка-строка станет `GlobalVariable` с типом `ptr`. Тогда она будет «переменной», и всё снова ок, но только случайно.

Посчитай, сколько раз зовётся `loadValueIfPointer` в кодгене (`grep -c`): это и есть число мест, где можно забыть.

### 6.3. Объявление переменной: `visit(VariableAssignNode)` → `Declarations` (`ASTGen.cpp:310`, `Declarations.cpp`)

```cpp
if (node.name == "_") context.NamedValues.erase("_");            // ①
if (NamedValues.find(node.name) != end()) { result = nullptr; return; } // ②
llvm::Type* varType;                                             // ③ НЕ инициализирован
if (node.inferredType) varType = getLLVMType(node.inferredType);
else if (node.type)    varType = getLLVMType(node.type);
bool isGlobalContext = GetInsertBlock()->getParent() == context.StartFunction; // ④
if (isGlobalContext) { result = handleGlobalVariable(...); return; }
if (Arrays::isLiteral(node.expression)) { result = handleArrayInitialization(...); return; } // ⑤
result = Declarations::handleSimpleAssignment(context, node, varType);
```

1. `_ ^= f()`: значение нужно вычислить, а имя `_` можно занимать много раз.
2. Повторное объявление **молча игнорируется**. Семантика это запрещает, так что сюда попасть не должно, но если попадёшь, то будет тихо (14.4).
3. Если нет ни `type`, ни `inferredType`, будет чтение неинициализированной переменной (UB).
4. Глобалка или локалка определяется по тому, где стоит курсор.
5. «Литерал» здесь это `[...]` / `{...}` (в AST это `BlockNode`) или пустой `[]` (`NoneNode` с `isEmptyLiteral`).

`handleSimpleAssignment`:

```cpp
ASTGen codeGen(context);                                         // ① новый визитор
if (hasNoValue(node)) { /* i32 x без значения → alloca + 0 / "" */ }
node.expression->accept(codeGen);
llvm::Value* value = codeGen.getResult();
...
llvm::Type* valueType = value->getType();                        // ② до проверки на null!
if (!value) { … return nullptr; }
value = loadValueIfPointer(context, value);                      // ③
value = convertValueToType(context, value, varType);             // ④
AllocaInst* alloca = createEntryAlloca(varType, node.name);      // ⑤
NamedValues[node.name] = alloca;
CreateStore(value, alloca);
```

1. Отдельный визитор только ради чистого `result`.
2. **Порядок перепутан:** `value->getType()` вызывается до проверки `if (!value)`. Если `value == nullptr`, будет segfault раньше, чем «красивая» ошибка. Переменная `valueType` к тому же нигде не используется.
3. Если справа переменная (`i32 y = x`), то загружаем.
4. Всегда приводим к типу переменной, иначе `store i64` в `alloca i32` даст невалидный IR.
5. alloca в entry-блоке (см. ниже).

**`createEntryAlloca` (в хедере):**

```cpp
llvm::Function* function = Builder.GetInsertBlock()->getParent();
llvm::IRBuilder<> entryBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
return entryBuilder.CreateAlloca(type, nullptr, name);
```

Все alloca ставятся **в начало entry-блока**, а не туда, где объявлена переменная. Иначе alloca внутри цикла выделял бы стек на каждой итерации. Плюс `mem2reg`/SROA в O2 умеют поднимать в регистры только alloca из entry-блока. Поэтому в IR alloca идут в обратном порядке (`%b2` перед `%a1`): каждая новая вставляется в самое начало.

### 6.4. Глобалки: `handleGlobalVariable` (`Declarations.cpp:76`)

```cpp
initValue = ...;                                          // вычислить в __ms_start
initValue = loadValueIfPointer(...); initValue = convertValueToType(...);
Constant* constantValue = dyn_cast<Constant>(initValue);  // ①
GlobalVariable* globalVar = new GlobalVariable(*TheModule, varType,
    node.isConst && constantValue,                        // ②
    PrivateLinkage,
    constantValue ? constantValue : Constant::getNullValue(varType), // ③
    node.name);
if (varType->isPointerTy() && !globalVar->isConstant())
    registerGCRoot(context, globalVar);                   // ④
if (!constantValue) CreateStore(initValue, globalVar);    // ⑤
```

1. Если значение получилось константой (литерал или константное выражение, которое IRBuilder свернул), то оно кладётся прямо в инициализатор.
2. Константная память только для `const` **и** константного значения.
3. Иначе пока ноль…
4. …GC должен знать про глобальную ссылку (под JIT он сам не найдёт сегменты данных модуля), поэтому `GC_add_roots(&g, &g + size)`…
5. …а настоящее значение пишется при старте.

---

## Часть 7. Управление потоком: if, while, for

### 7.1. Пример 02: исходник

```
[i32]main()
|   i32 sum = 0
|   for i in range(5)
|   |   if i == 3
|   |   |   continue
|   |   sum = sum + i
|   while sum > 0
|   |   sum = sum - 4
|   return sum
```

Граф блоков, который получился:

```
            stack.ok
               │  sum = 0; i = 0
               ▼
       ┌──▶ for.cond ── i < 5 ? ──no──▶ for.end ──▶ loop.cond ◀──┐
       │       │yes                                     │        │
       │       ▼                                        │ sum>0? │
       │    for.body ── i == 3 ?                        │yes     │
       │     │yes          │no                          ▼        │
       │     ▼             ▼                         loop.body ──┘
       │   then         ifcont  sum = sum + i           │no (из loop.cond)
       │ (continue)        │                            ▼
       │     │             │                         loop.end → ret sum
       │     ▼             ▼
       └── for.step ◀──────┘   i = i + 1
```

Реальный IR (без пролога):

```llvm
stack.ok:
  store i32 0, ptr %sum, align 4
  store i32 0, ptr %i, align 4                     ; range(5) это range(0, 5)
  br label %for.cond

for.cond:
  %for.current = load i32, ptr %i, align 4
  %for.check = icmp slt i32 %for.current, 5        ; граница посчитана ОДИН раз, до цикла
  br i1 %for.check, label %for.body, label %for.end

for.body:
  %left_load = load i32, ptr %i, align 4
  %cmptmp = icmp eq i32 %left_load, 3
  br i1 %cmptmp, label %then, label %ifcont

for.step:                                          ; сюда идёт continue
  %for.step.current = load i32, ptr %i, align 4
  %for.next = add i32 %for.step.current, 1
  store i32 %for.next, ptr %i, align 4
  br label %for.cond

for.end:
  br label %loop.cond

then:                                              ; тело if: только continue
  br label %for.step

ifcont:
  %left_load1 = load i32, ptr %sum, align 4
  %right_load = load i32, ptr %i, align 4
  %addtmp_int = add i32 %left_load1, %right_load
  store i32 %addtmp_int, ptr %sum, align 4
  br label %for.step

loop.cond:                                         ; while
  %left_load2 = load i32, ptr %sum, align 4
  %cmptmp3 = icmp sgt i32 %left_load2, 0
  br i1 %cmptmp3, label %loop.body, label %loop.end

loop.body:
  %left_load4 = load i32, ptr %sum, align 4
  %subtmp_int = sub i32 %left_load4, 4
  store i32 %subtmp_int, ptr %sum, align 4
  br label %loop.cond

loop.end:
  %return_load_load = load i32, ptr %sum, align 4  ; имя "return_load" + "_load": loadValueIfPointer добавил суффикс
  ret i32 %return_load_load
```

Порядок блоков в файле (`for.step` до `then`) такой, потому что блоки создаются **заранее** (`BasicBlock::Create(…, function)` добавляет в конец функции в момент создания), а заполняются позже. На корректность это не влияет.

### 7.2. `visit(IfNode)` (`ASTGen.cpp:407`)

```cpp
node.condition->accept(*this);
condValue = loadValueIfPointer(context, getResult(), "condition");
if (!condValue->getType()->isIntegerTy(1))
    condValue = CreateICmpNE(condValue, getNullValue(type), "ifcond");  // ①

then  = Create("then"); merge = Create("ifcont");
else_ = node.elseBlock ? Create("else") : nullptr;
CreateCondBr(condValue, then, else_ ? else_ : merge);

auto outerNamedValues = context.NamedValues;                            // ②
SetInsertPoint(then);
node.thenBlock->accept(*this);
if (!GetInsertBlock()->getTerminator()) CreateBr(merge);                // ③
context.NamedValues = outerNamedValues;
// то же для else
SetInsertPoint(merge);
```

1. `if n` где `n: i32` превращается в `n != 0`. Truthiness.
2. Переменные ветки живут только в ветке.
3. `GetInsertBlock()`, а **не** `then`: тело могло создать свои блоки (вложенный if, проверку деления), и `br` нужен из последнего. Если ветка закончилась `return`/`continue`, терминатор уже есть, и `br` не нужен.

Есть тонкость: `merge` создаётся **всегда**, даже если обе ветки сделали `return`. Тогда `ifcont` остаётся без предшественников, и в него пишется код после if. Это валидный IR (недостижимый блок), O2 его выкинет. Но если после такого if в функции нет кода, то `ifcont` останется **без терминатора**, и это спасает 10-й шаг `visit(FunctionNode)`, который туда допишет `unreachable`.

### 7.3. `visit(ForNode)`: четыре вида циклов (`ASTGen.cpp:565`)

```cpp
auto range = dynamic_pointer_cast<CallNode>(node.iterable);
if (!range || range->callee != "range") {
    if (iterable type == "string") emitStringFor(...);   // for c in s
    else if (isMap(...))           Maps::emitFor(...);   // for k in m
    else                           emitArrayFor(...);    // for x in a
    return;
}
// for i in range(a, b)
```

| Вид | `continue` идёт в | Особенности |
|---|---|---|
| `range` | `for.step` | границы считаются один раз, слева направо; `range(n)` = `range(0, n)`; сравнение **знаковое** `slt` |
| массив | `for.step` | длина запоминается до цикла; **на каждой итерации проверяется, что длина не изменилась**, иначе паника `"array changed during iteration"`; `x` это **копия** элемента |
| строка | `for.step` | идёт по **байтам** до `\0`, а не по длине |
| map | `for.cond` | всё делает рантайм `ms_map_next` (версия map'а проверяется там же) |

`emitArrayFor` внутри (`ASTGen.cpp:463`):

```
  array = <iterable>;  length = array.len;  index = 0
  ┌─▶ for.cond:
  │     if array.len != length → for.changed: panic("array changed during iteration")
  │     for.same: if index < length → body, else → end
  │   for.body:
  │     x = array.data[index]     ← без проверки границ: длина та же, индекс < длины
  │     <тело>
  │   for.step:
  └──── index++
      for.end
```

Хорошо продумано: проверка «массив не меняется» и позволяет безопасно убрать bounds-check в теле.

### 7.4. `break`/`continue` (`ASTGen.cpp:1070`)

`pushLoopContext(condBlock, endBlock)` кладёт блоки в два стека. `break` → `br` на верх `loopEndBlocks`, `continue` → `br` на верх `loopCondBlocks`. Что лежит в «cond», решает сам цикл: для `for` это `for.step`, для `while` это `loop.cond`.

⚠️ Есть одна хрупкость: `pushLoopContext` пушит, **только если блок не null**, а `popLoopContext` попает **оба, если не пусты**. Сейчас все циклы передают оба блока, и всё сходится. Но если когда-нибудь передашь `nullptr` в один, то стеки разъедутся.

⚠️ И ещё: `break`/`continue` внутри `attempt` внутри цикла просто прыгнут из `attempt` наружу, а стек `PanicHandlers` откатится на этапе **генерации** (`pop_back` после тела). Это корректно, потому что обработчики разрешаются статически. Просто знай, что это работает именно поэтому.

---

## Часть 8. Выражения: операторы и касты

### 8.1. Короткое замыкание `and`/`or`: `visit(BinaryOpNode)` (`ASTGen.cpp:895`)

```cpp
node.left->accept(*this);
if (node.op == "and" || node.op == "or") {
    Value* leftBool = toBool(left, node.left);
    BasicBlock* leftBlock  = GetInsertBlock();                 // ①
    BasicBlock* rightBlock = Create(node.op + ".right");
    BasicBlock* doneBlock  = Create(node.op + ".done");
    if (and) CreateCondBr(leftBool, rightBlock, doneBlock);    // ②
    else     CreateCondBr(leftBool, doneBlock, rightBlock);
    SetInsertPoint(rightBlock);
    node.right->accept(*this);
    Value* rightBool = toBool(getResult(), node.right);
    BasicBlock* rightEnd = GetInsertBlock();                   // ③
    CreateBr(doneBlock);
    SetInsertPoint(doneBlock);
    PHINode* value = CreatePHI(i1, 2, node.op);
    value->addIncoming(getInt1(node.op == "or"), leftBlock);   // ④
    value->addIncoming(rightBool, rightEnd);
}
```

1. Блок, **из которого** пришли слева. Левая часть тоже могла создать блоки, поэтому берётся текущий, а не какой-то запомненный раньше.
2. `and`: левая ложь → сразу `done`. `or`: левая истина → сразу `done`.
3. **Классическая ловушка с phi**, обойдённая правильно: правая часть могла создать блоки (`a[j-1]` создаёт `bounds.ok`), и входящий блок для phi это **последний** блок правой части, а не `rightBlock`.
4. Если пришли сразу из левой части, то результат известен: `false` для and, `true` для or.

```
   left ──(and: false)──────────────┐
    │ true                          ▼
    ▼                         and.done: phi [false, left], [rightBool, rightEnd]
  and.right → … bounds.ok ──────────┘
                 (rightEnd)
```

### 8.2. `handleBinaryOperation` (`Expressions.cpp:48`)

Структура функции:

```
 1. load обоих операндов (loadValueIfPointer)
 2. applyImplicitCast для каждого, если семантика сказала
 3. «добивка» типов, если всё ещё разные:
      int + int   → к большей ширине
      float + f64 → к f64
      int + fp    → int в тип fp
 4. диспатч по node.op (строки, которые выдала семантика):
      add/fadd, sub/fsub, mul/fmul       → Add / FAdd …
      sdiv/fdiv                          → проверка на 0, проверка MIN/-1, SDiv
      srem/frem                          → проверка на 0, подмена делителя -1→1, SRem
      scat                               → вызов scat из stdlib (TOML)
      and/or                             → ⚠ мёртвые, их ловит visit(BinaryOpNode)
      strcmp_eq/ne/slt/…                 → strcmp(l, r) <op> 0
      icmp_eq/ne/slt/sgt/sge/sle         → ICmp
      fcmp_eq/ne/slt/…                   → FCmp (O = ordered, ne = UNE)
```

Пара мест, которые стоит прочитать внимательно:

**Деление** (`sdiv`):

```cpp
emitDivisionByZeroCheck(context, right, node.line);            // ①
if (left->getType()->isIntegerTy(1))
    return CreateUDiv(left, right, "divtmp_bool");             // ②
emitDivisionOverflowCheck(context, left, right, node.line);    // ③
return CreateSDiv(left, right, "divtmp_int");
```

1. `x / 0` → паника.
2. `i1 / i1`: знаковое деление в i1 трактует `1` как `-1`, и `1/1` было бы «переполнением». Поэтому беззнаковое.
3. `INT_MIN / -1` → паника `"integer overflow"` (в LLVM это UB, а на x86 это реальный `SIGFPE`).

**Остаток** (`srem`): вместо проверки `MIN % -1` делитель `-1` подменяется на `1` через `select`. Остаток от деления на ±1 всегда 0, так что результат тот же, но без лишнего блока. Изящно.

**Сравнение float**: `fcmp_ne` → `FCMP_UNE` (unordered, то есть `NaN != NaN` → true), остальные **ordered** (`NaN == NaN` → false). Это IEEE-корректно.

Пример: `1 / 0` с литералами (пример 06):

```llvm
stack.ok:
  br i1 true, label %div.zero, label %div.ok       ; ← IRBuilder свернул icmp eq 0, 0 в true!

div.zero:
  call void @ms_raise(ptr …"division by zero"…, i32 3, ptr %panic.context)
  br label %attempt.panic

div.ok:
  br i1 false, label %div.overflow, label %div.safe

div.safe:
  store i8 poison, ptr %n, align 1                 ; ← sdiv 1, 0 IRBuilder свернул в poison
  br label %attempt.end
```

Интересный момент: `IRBuilder` **сам** сворачивает константы. Проверка на ноль стала `br i1 true`, а само деление `poison`. Код корректный, потому что до `div.safe` выполнение никогда не дойдёт. Но если когда-нибудь уберёшь проверку и оставишь только `sdiv`, то `poison` молча утечёт в программу. Ещё один повод не убирать проверки.

### 8.3. `convertValueToType` (`TypeConversions.cpp:98`): таблица

| из \ в | `i1` | `iN` | `float/f64` | `ptr` |
|---|---|---|---|---|
| `i1` | — | `zext` (0/1) | `uitofp` (иначе true = -1.0) | — |
| `iN` | `icmp ne 0` (иначе `trunc` сделает 2 → false) | `sext`/`trunc` (`CreateIntCast` signed) | `sitofp` | — |
| `float/f64` | проверка NaN, `fcmp une 0` | **проверка диапазона**, `fptosi` | `fpext`/`fptrunc` (литерал вне float → ошибка компиляции) | — |
| `ptr` | — | — | — | `bitcast` (с opaque pointers это no-op) |

Всё, что не попало в таблицу, возвращается **как есть, молча**. Например, `i32` → `ptr`. Это опять тихий фолбэк (14.4).

### 8.4. `emitFloatToInt` (`TypeConversions.cpp:14`)

```cpp
Value* truncated = CreateUnaryIntrinsic(Intrinsic::trunc, value);  // ①
if (width == 1) fits = CreateFCmpOEQ(value, value);                 // ②
else {
    const double limit = std::ldexp(1.0, width - 1);                // ③ 2^(N-1)
    fits = CreateAnd(CreateFCmpOGE(truncated, -limit),
                     CreateFCmpOLT(truncated,  limit));             // ④
}
CreateCondBr(fits, okBlock, errorBlock);
// error: panic("float value out of range for iN")
// ok:
if (width == 1) return CreateFCmpUNE(truncated, 0.0);               // ⑤
return CreateFPToSI(value, targetType);
```

1. Отбросить дробную часть **до** сравнения: `-128.9` → `-128`, и это влезает в `i8`.
2. Для `i1` проверяется только «не NaN» (`x == x` ложно только для NaN).
3. Границы `[-2^(N-1), 2^(N-1))` это ровно диапазон знакового `iN`, и они точно представимы в double.
4. Ordered-сравнения: при NaN оба ложны, и NaN тоже уходит в панику. Одна проверка ловит и переполнение, и NaN.
5. `fptosi` в `i1` дал бы -1, поэтому `!= 0`.

`applyCharCast`: `char` хранится как `i8`, а «беззнаковость» добавляется здесь. Если узел типа `char` и семантика сказала привести его к целому шире `i8`, то делается `zext` (а не `sext`, который сделал бы `convertValueToType`). Вызывается **в каждом месте, где появляется значение** (`IdentifierNode`, `CallNode`, `AccessExpression`, `pop`, `try`). Это прямое следствие проблемы 6.2: нет одной точки «получить rvalue», куда это можно было бы положить.

---

## Часть 9. Вызовы: `visit(CallNode)` (`ASTGen.cpp:691`)

### 9.1. Схема лесенки

```
 visit(CallNode)
   calleeFunc = TheModule->getFunction(callee)
   │
   ├─ node.functionValueType?  ── да ──▶ вызов func<>-значения по адресу ───────▶ return
   │
   ├─ !calleeFunc && "len"     ──▶ ms_str_len / ms_map_len / array.length ───────▶ return
   ├─ !calleeFunc && "cstr"    ──▶ сам аргумент (строка) ─────────────────────────▶ return
   ├─ !calleeFunc && ok / err  ──▶ GC_malloc result, заполнить поля ──────────────▶ return
   ├─ !calleeFunc && push/pop  ──▶ ms_array_reserve + store / проверка пустоты ────▶ return
   ├─ !calleeFunc && has/del   ──▶ ms_map_has / ms_map_del ─────────────────────────▶ return
   ├─ !calleeFunc && copy      ──▶ Objects::emitCopy ───────────────────────────────▶ return
   ├─ !calleeFunc && Structs[callee] ──▶ Objects::emitNew (P()) ────────────────────▶ return
   │
   ├─ !calleeFunc ──▶ declareFunctionFromTOML(callee)    ← stdlib
   ├─ всё ещё нет ──▶ throw "unknown function"
   ├─ число аргументов не совпало ──▶ ⚠ ТИХО result = nullptr
   │
   └─ обычный вызов:
        для каждого аргумента: литерал? → emitLiteral, иначе accept
                                load → applyImplicitCast → convertValueToType(тип параметра)
        emitCall(calleeFunc, args)  → applyCharCast
```

`!calleeFunc &&` перед каждым встроенным нужно, чтобы **пользовательская функция с таким же именем побеждала** (семантика решает так же). Если ты напишешь свою `len`, то вызовется она.

### 9.2. `push` построчно

```cpp
auto arrayType = dynamic_pointer_cast<GenericTypeNode>(node.arguments[0]->inferredType);
Type* elementType = getLLVMType(arrayType->typeParameters[0]);
node.arguments[0]->accept(*this);
Value* array = loadValueIfPointer(context, getResult());
Value* lengthField = CreateStructGEP(header, array, 1);

Value* value = Arrays::emitValue(context, node.arguments[1], elementType); // ①
CreateCall(ms_array_reserve, {array, getInt64(elementSize),
                              getInt32(elementType->isPointerTy())});      // ②
Value* length = CreateLoad(i32, lengthField);                              // ③
CreateStore(value, CreateGEP(elementType, getArrayData(array), length));   // ④
CreateStore(CreateAdd(length, 1), lengthField);
```

1. Значение вычисляется **до** reserve, потому что это `push(a, f())`, и `f` может сама менять `a`.
2. `ms_array_reserve` вызывается через `CreateCall`, **не через `emitCall`**, и он не в списке «паникующих». Значит, если рантайм когда-нибудь начнёт паниковать там (например, при переполнении размера), то паника потеряется.
3. Длина читается **после** reserve, а то вдруг поменялась.
4. `getArrayData` тоже **после** reserve: данные могли переехать. Комментарий в коде это и говорит.

IR (пример 03, `push(a, 30)`):

```llvm
  %load = load ptr, ptr %a, align 8
  %6 = getelementptr inbounds %ms_array, ptr %load, i32 0, i32 1      ; &a.length (до reserve, но это адрес заголовка, он не переезжает)
  call void @ms_array_reserve(ptr %load, i64 4, i32 0)                 ; 4 байта элемент, не указатель
  %length = load i32, ptr %6, align 4
  %7 = getelementptr inbounds %ms_array, ptr %load, i32 0, i32 0
  %array_data3 = load ptr, ptr %7, align 8                             ; data ПОСЛЕ reserve
  %8 = getelementptr i32, ptr %array_data3, i32 %length
  store i32 30, ptr %8, align 4
  %9 = add i32 %length, 1
  store i32 %9, ptr %6, align 4
```

### 9.3. Вызов `func<>`-значения

```cpp
auto type = dynamic_pointer_cast<GenericTypeNode>(node.functionValueType); // func<A, B, R>
for (i = 0; i + 1 < type->typeParameters.size(); i++) {                     // ①
    parameterTypes.push_back(getLLVMType(type->typeParameters[i]));
    arguments.push_back(Arrays::emitValue(context, node.arguments[i], type->typeParameters[i]));
}
Type* returnType = getLLVMType(type->typeParameters.back());               // ②
Value* target = CreateLoad(ptr, NamedValues.at(node.callee), "function");  // ③
parameterTypes.push_back(ptr); arguments.push_back(PanicContext);          // ④
Value* call = CreateCall(FunctionType::get(returnType, parameterTypes, false), target, arguments);
checkPanic();
```

1. Все параметры кроме последнего это аргументы…
2. …а последний это тип результата.
3. Переменная хранит адрес функции, его надо загрузить. Тут `NamedValues.at` **кинет `std::out_of_range`**, если имени нет. Это ещё один, пятый, способ сообщить об ошибке.
4. Скрытый аргумент добавляется руками. Поэтому все `func<>`-значения обязаны быть с этим соглашением, отсюда `functionAdapter`.

---

## Часть 10. Цепочки доступа

### 10.1. Как семантика представляет `a[i].f`

`AccessExpression` это **связный список**. У корня `expression` это база (`a`), `nextAccess` это следующее звено. У каждого звена `notation` (`"[]"` или `"."`) говорит, **как получить следующее** звено, и у следующего звена лежит индекс (`expression`) или имя (`baseName`) и тип результата (`inferredType`).

```
 a[i].f.get()

 root ──────────────▶ link1 ─────────────▶ link2 ─────────────▶ link3
 expression = a       expression = i       baseName = "f"       expression = CallNode get()
 notation = "[]"      notation = "."       notation = "."
 inferredType: arr<P> inferredType: P      inferredType: Q      inferredType: i32
```

### 10.2. `Arrays::emitElementPtr` (`Arrays.cpp:71`)

```cpp
root.expression->accept(codeGen);
Value* base = loadValueIfPointer(context, codeGen.getResult());       // ① значение базы
AccessExpression* link = &root;
shared_ptr<TypeNode> currentType = root.expression->inferredType;
Value* elementPtr = nullptr;
while (auto next = dynamic_pointer_cast<AccessExpression>(link->nextAccess)) {
    if (elementPtr)
        base = CreateLoad(ptr, elementPtr, "nested_object");          // ② не первое звено
    elementType = getLLVMType(next->inferredType);

    if      ("." && currentType is result)     { /* .value с проверкой ok / .ok / .code / .error */ }
    else if ("." && currentType is panic_info) { /* .line / .message */ }
    else if ("[]" && isMap(currentType))       { /* Maps::emitSlot(createKey = forWrite && последнее) */ } // ③
    else if ("[]" && string)                   { /* ms_str_at → ptr на байт */ }
    else if ("[]")                             { /* getArrayElementPtr с проверкой границ */ }
    else {                                     // "." на структуре
        if (вызов метода) {
            returned = emitCall(P.method, {base, args…});
            elementPtr = createEntryAlloca(elementType, "method_result");  // ④
            CreateStore(returned, elementPtr);
        } else
            elementPtr = CreateStructGEP(layout.type, base, fieldIndex[name]);
    }
    currentType = next->inferredType;
    link = next.get();
}
return elementPtr;                                                    // ⑤ АДРЕС последнего
```

1. База загружается **один раз**.
2. Ключевая строчка. На каждом шаге, кроме первого, «то, что лежит по адресу предыдущего звена» это ссылка на следующий объект. Загружается как `ptr`. Работает, потому что все промежуточные звенья (массив, структура, map, result) это ссылки. Если когда-нибудь появятся структуры **по значению** внутри структур, то эта строчка сломается первой.
3. В `m[k][0] = x` ключ `k` должен **уже существовать** (иначе паника), а в `m[k] = x` он создаётся. Это решается тем, что `createKey` ставится только для последнего звена.
4. Результат метода кладётся во **временную alloca**, чтобы цепочка шла дальше «как обычно» (через `CreateLoad(ptr, elementPtr)` на следующем шаге, или через `load` в `visit(AccessExpression)`). Это и есть «значение, которое является alloca» из 6.2.
5. Всегда **адрес**. Чтение (`visit(AccessExpression)`) делает `load`, а запись (`visit(ReassignMemberNode)`) делает `store`. Это уже почти lvalue-модель, только локальная для цепочек. Именно её стоит распространить на всё (часть 15).

### 10.3. Пример 04: структура и метод

```
[struct]P
|   i32 age = 1
|   [void]birthday()
|   |   self.age = self.age + 1

[i32]main()
|   P p = P()
|   p.birthday()
|   return p.age
```

IR (без прологов и паник-проверок):

```llvm
%ms_struct_P = type { i32 }

define private ptr @__ms_new_P(ptr %0) #0 {               ; конструктор
entry:
  %object = call ptr @GC_malloc_atomic(i64 4)             ; нет полей-указателей → atomic
  %1 = getelementptr inbounds %ms_struct_P, ptr %object, i32 0, i32 0
  store i32 1, ptr %1, align 4                             ; age = 1 (значение по умолчанию)
  ret ptr %object
}

define internal void @P.birthday(ptr %self, ptr %panic.context) #0 {
entry:
  %self1 = alloca ptr, align 8
  store ptr %self, ptr %self1, align 8
  ; …пролог…
stack.ok:
  %load = load ptr, ptr %self1, align 8                    ; self (значение)
  %age = getelementptr inbounds %ms_struct_P, ptr %load, i32 0, i32 0
  %element = load i32, ptr %age, align 4                   ; self.age (чтение)
  %addtmp_int = add i32 %element, 1
  %load2 = load ptr, ptr %self1, align 8                   ; self ЕЩЁ РАЗ: сначала значение, потом место
  %age3 = getelementptr inbounds %ms_struct_P, ptr %load2, i32 0, i32 0
  store i32 %addtmp_int, ptr %age3, align 4                ; self.age = … (запись)
  ret void
}

define internal i32 @__ms_user_main(ptr %panic.context) #0 {
  ; …
  %object = call ptr @__ms_new_P(ptr %panic.context)       ; P()  + checkPanic
  ; …
  store ptr %object, ptr %p, align 8
  %load = load ptr, ptr %p, align 8
  call void @P.birthday(ptr %load, ptr %panic.context)     ; p.birthday() + checkPanic
  ; …
  %load3 = load ptr, ptr %p, align 8
  %age = getelementptr inbounds %ms_struct_P, ptr %load3, i32 0, i32 0
  %element = load i32, ptr %age, align 4                   ; return p.age
  ret i32 %element
}
```

Заметь: у конструктора **нет** пролога с проверкой стека. При этом он `"ms.panic_context"`, так что если значение по умолчанию это `1 / 0`, то `emitPanic` сделает правильный `ret null` (по ветке ② в 4.4). Рекурсивный конструктор (`P` с полем `P inner = P()`) мог бы уйти в бесконечную рекурсию без паники `stack overflow`, но **семантика это уже запрещает** (проверил: `Struct 'P' cannot be created: its fields always create another P`). Так что сейчас это безопасно. Помни про это, если когда-нибудь ослабишь проверку (например, ради ленивых полей).

`ReassignMemberNode` (`self.age = …`): **сначала значение, потом место** (комментарий «как в Python»). Поэтому `self` загружается два раза. Это порядок вычисления, который ты выбрал, и он правильный для `a[f()] = g()`. Запиши его в спеку.

---

## Часть 11. Структуры и map

### 11.1. `Objects` (`Objects.cpp`)

- `registerStruct`: проход по телу. `FunctionNode` это метод, `VariableAssignNode` это поле. `fieldIndex[name] = i`, `hasReferences |= поле указатель`. `StructType::create(..., "ms_struct_" + name)` это **именованный** тип (`%ms_struct_P`), его видно в IR. Конструктор создаётся пустым, а тело дописывается позже.
- `emitConstructor`: подменяет `PanicContext` на аргумент конструктора, **очищает** `NamedValues` (значения по умолчанию не видят локалок), выделяет память и для каждого поля вычисляет значение по умолчанию **при каждом вызове** `P()`. То есть `array<i32> a = []` в поле означает новый массив на каждый объект, а не общий. Правильная семантика (в Python тут известная ловушка, у тебя её нет).
- `emitCopy`: **поверхностная** копия. Для массива это новый заголовок + `memcpy` данных (ссылки внутри общие), для структуры это `memcpy` всего объекта, для map это `ms_map_copy`.

⚠️ `emitCopy` для структуры без ссылок делает `GC_malloc_atomic` + `memcpy`. Ок. Для массива `createArray` выделяет atomic-данные (не обнулены) и тут же их полностью перезаписывает. Тоже ок. Но обрати внимание: если `length == 0`, то `memcpy` на 0 байт с указателем, который вернул `GC_malloc_atomic(0)`. Это валидно, просто знай.

### 11.2. `Maps` (`Maps.cpp`)

Map целиком в рантайме, кодген только вызывает функции:

| Операция | Рантайм | Паникует |
|---|---|---|
| литерал / `map<K,V> m = {}` | `ms_map_new(keyIsString, hasReferences)` | — |
| `m[k] = v` | `ms_map_insert(m, key)` → адрес 8-байтовой ячейки | — |
| `m[k]` | `ms_map_get(m, key, line)` → адрес ячейки | ✔ нет ключа |
| `len(m)` | `ms_map_len` | — |
| `has(m, k)` | `ms_map_has` | — |
| `del(m, k)` | `ms_map_del(m, key, line)` | ✔ нет ключа |
| `for k in m` | `ms_map_version` + `ms_map_next(m, &pos, version, line, &key)` | ✔ map изменился |
| `copy(m)` | `ms_map_copy` | — |

Ключ всегда приводится к `i64` (`emitKey`): строка через `ptrtoint` (ключ это **адрес**, и рантайм с `keyIsString = 1` хеширует содержимое, а не адрес), `i1` через `zext`, остальные целые через `sext`.

⚠️ `for k in m` кладёт ключ в `i64`-слот и потом читает его как `keyType` (`CreateLoad(keyType, keySlot)`). Для `i32`-ключа это чтение младших 4 байт `i64`. **Работает только на little-endian.** x86 и ARM (обычно) это little-endian, так что сейчас неважно, но это неявное допущение. Правильнее было бы `trunc` после загрузки `i64`.

---

## Часть 12. `result<T>`, `?` и `attempt`

### 12.1. Раскладка

```
 result<i32>   = { i8 ok, i32 code, ptr error, i32 value }    ← в куче, переменная = ptr
 result<void>  = { i8 ok, i32 code, ptr error }
```

`getResultStruct` создаёт **анонимную** структуру (`StructType::get`), в отличие от `%ms_array`/`%ms_struct_P`. Поэтому в IR она видна как `{ i8, i32, ptr, i32 }`.

`ok(x)`: `GC_malloc`, `ok = 1`, `code = 0`, `error = ""`, `value = x`.
`err(code, msg)` / `err(msg)`: `ok = 0`, `code`, `error = msg`, а `value` **не трогается**: память от `GC_malloc` обнулена (не atomic, потому что там `ptr error`).

### 12.2. `r?` (`Expressions.cpp`, ветка `try`)

Пример 05:

```
[result<i32>]half(i32: n)
|   if n % 2 != 0: return err(1, "odd")
|   return ok(n / 2)

[result<i32>]quarter(i32: n)
|   i32 h = half(n)?
|   return half(h)
```

IR функции `quarter` (без пролога):

```llvm
stack.ok:
  %arg_load = load i32, ptr %n1, align 4
  %calltmp = call ptr @half(i32 %arg_load, ptr %panic.context)
  ; ── checkPanic ──
  %3 = load i8, ptr %2, align 1
  %4 = icmp ne i8 %3, 0
  br i1 %4, label %panic.propagate, label %panic.clear

panic.propagate:
  ; …line…
  ret ptr null                                         ; ← паника: result-функция возвращает null

panic.clear:
  ; ── r? ──
  %9 = getelementptr inbounds { i8, i32, ptr, i32 }, ptr %calltmp, i32 0, i32 0
  %10 = load i8, ptr %9, align 1                       ; ok?
  %11 = icmp ne i8 %10, 0
  br i1 %11, label %try.ok, label %try.error

try.ok:
  %12 = getelementptr inbounds { i8, i32, ptr, i32 }, ptr %calltmp, i32 0, i32 3
  %unwrapped = load i32, ptr %12, align 4              ; value
  store i32 %unwrapped, ptr %h, align 4
  ; … return half(h) …

try.error:                                             ; режим 4: пробросить ошибку наверх
  %17 = load i32, ptr %16, align 4                     ; code
  %19 = load ptr, ptr %18, align 8                     ; error
  %propagated_error = call ptr @GC_malloc(i64 24)      ; НОВЫЙ result в раскладке ЭТОЙ функции
  store i8 0, ptr %20, align 1
  store i32 %17, ptr %21, align 4
  store ptr %19, ptr %22, align 8
  ret ptr %propagated_error
```

Почему новый объект, а не `ret ptr %calltmp`: `half` и `quarter` могут возвращать **разные** `result<A>` и `result<B>`. Раскладки разные (размер `value`), так что переиспользовать объект нельзя. Тут они совпадают (`result<i32>`), и это лишняя аллокация. Оптимизация на будущее: если `T` совпадает, возвращать исходный объект.

Четыре режима `?` (порядок проверок в коде):

```
 r?               ┌── fallback есть (r ? default)      → phi(value, default)
   ok? ── нет ──▶ ├── errorBody есть                   → тело с переменной error, потом unreachable
     │ да         ├── мы в attempt с error-веткой      → store r в слот, br attempt.error
     ▼            └── иначе                            → новый result(code, error), ret
   value
```

⚠️ В режиме `errorBody` (`r? else: …`) используется `ASTGen gen(context)` и **жёстко** имя переменной `"error"`. Если тело не завершилось, то `CreateUnreachable()`. То есть «ты обязан выйти из тела». Семантика это **проверяет** (проверил: `Local action handler must end with return, break or continue`), так что `unreachable` тут законный. Связь неявная: кодген полагается на правило семантики, и это стоит записать комментарием рядом с `CreateUnreachable`.

### 12.3. `attempt` (`ASTGen.cpp:1121`)

Пример 06:

```
[i32]main()
|   attempt
|   |   n ^= 1 / 0
|   panic info
|   |   echo(info.message)
|   return 0
```

```cpp
auto merge = Create("attempt.end");
auto error = node.errorBody ? Create("attempt.error") : nullptr;
auto panic = node.panicBody ? Create("attempt.panic") : nullptr;
auto errorSlot = error ? createEntryAlloca(ptr, "attempt.error.value") : nullptr;
auto names = context.NamedValues;
if (error) context.ResultHandlers.push_back({error, errorSlot});   // ①
if (panic) context.PanicHandlers.push_back(panic);
node.body->accept(*this);                                          // ②
if (!GetInsertBlock()->getTerminator()) CreateBr(merge);
if (error) context.ResultHandlers.pop_back();                      // ③
if (panic) context.PanicHandlers.pop_back();
context.NamedValues = names;
// error-ветка: переменная = errorSlot, тело, br merge
// panic-ветка:
SetInsertPoint(panic);
auto info = allocateObject(sizeof(PanicContext), true, "panic.info");   // ④
auto snapshot = CreateLoad(layout, PanicContext);
CreateStore(snapshot, info);
CreateStore(CreateInsertValue(Constant::getNullValue(layout),
            CreateExtractValue(snapshot, 3), 3), PanicContext);         // ⑤
// переменная info = ptr на копию, тело, br merge
SetInsertPoint(merge);
```

1. Обработчики видны всему, что генерируется **внутри тела**: `propagatePanic` и `?` смотрят на верх стека.
2. Тело.
3. После тела обработчики снимаются. Код в самих обработчиках их **уже не видит**, поэтому паника в `panic`-ветке летит во **внешний** `attempt` (тест `handling_panic_nested.ms` это проверяет).
4. Копия контекста в куче, чтобы `info` пережил следующие паники (если её сохранить в переменную снаружи).
5. Контекст сбрасывается в ноль, **кроме `stackLimit`**. Если обнулить и его, после первого пойманного переполнения стека проверка стека навсегда отключится (`sp < null` всегда ложно).

IR panic-ветки:

```llvm
attempt.panic:                                        ; preds = %div.overflow, %div.zero
  %panic.info = call ptr @GC_malloc(i64 24)
  %2 = load { i8, i32, ptr, ptr }, ptr %panic.context, align 8
  store { i8, i32, ptr, ptr } %2, ptr %panic.info, align 8          ; снимок
  %3 = extractvalue { i8, i32, ptr, ptr } %2, 3                     ; stackLimit
  %4 = insertvalue { i8, i32, ptr, ptr } zeroinitializer, ptr %3, 3
  store { i8, i32, ptr, ptr } %4, ptr %panic.context, align 8       ; сброс, лимит сохранён
  store ptr %panic.info, ptr %info, align 8
  %load = load ptr, ptr %info, align 8
  %5 = getelementptr inbounds { i8, i32, ptr, ptr }, ptr %load, i32 0, i32 2
  %element = load ptr, ptr %5, align 8                              ; info.message
  call void @echo(ptr %element)
  br label %attempt.end
```

Обрати внимание: `div.zero` и `div.overflow` прыгают **прямо** в `attempt.panic` (`br`), без `ret` и без `checkPanic`. Это ветка ① в `propagatePanic`.

---

## Часть 13. Сколько это стоит после O2

Пример 01 (`add(2, 3)`) после O2. `add` и `__ms_user_main` заинлайнились в `main`, `2 + 3` свернулось в `5`… но смотри, что **осталось**:

```llvm
define noundef i32 @main() local_unnamed_addr {
entry:
  %panic.context = alloca { i8, i32, ptr, ptr }, align 8
  ; … обнуление, ms_rt_init, ms_stack_init …
  %stack.limit.i = load ptr, ptr %.fca.3.gep, align 8
  %sp.i = call ptr @llvm.stacksave.p0()                ; проверка стека __ms_user_main
  %0 = icmp ult ptr %sp.i, %stack.limit.i
  br i1 %0, label %__ms_user_main.exit, label %stack.ok.i

stack.ok.i:
  %stack.limit.i.i = load ptr, ptr %.fca.3.gep, align 8
  %sp.i.i = call ptr @llvm.stacksave.p0()              ; проверка стека add (заинлайненного!)
  %1 = icmp ult ptr %sp.i.i, %stack.limit.i.i
  br i1 %1, label %stack.overflow.i.i, label %add.exit.i

add.exit.i:
  %common.ret.op.i.i = phi i32 [ 0, %stack.overflow.i.i ], [ 5, %stack.ok.i ]   ; ← вот она, пятёрка
  %2 = load i8, ptr %panic.context, align 8            ; checkPanic после add
  %.not.i = icmp eq i8 %2, 0
  br i1 %.not.i, label %panic.clear, label %__ms_user_main.exit.thread
  ; … ещё ~25 строк паник-веток …
}
```

Что видно:

- **Проверка стека не выкидывается даже после инлайна.** `llvm.stacksave` для оптимизатора это «вызов с эффектом», и выкинуть его он не может. Каждая заинлайненная функция оставляет свою проверку. В горячем цикле с маленькими функциями это будет заметно.
- **Флаг паники перечитывается** после каждого вызова: оптимизатор не знает, что `add` не могла его поставить (она передаёт `%panic.context` в `ms_raise`, и он «утекает»).
- Строки `"stack overflow"` не склеились.

Это не баги, это **цена модели**. Варианты удешевления на будущее (решения твои):
- не ставить проверку стека в функции без вызовов (листовые не могут переполнить стек рекурсией);
- ставить её только в функции, которые участвуют в цикле вызовов (рекурсия): это анализ графа вызовов, его можно сделать до кодгена;
- пометить рантайм-функции, которые **не** паникуют, атрибутами (`nounwind`, `memory(argmem: …)`), чтобы оптимизатор мог доказать, что флаг не изменился.

---

## Часть 14. Где говно

Проверено на сборке: **все 965 тестов проходят**, а неоптимизированный IR всех 454 run-тестов, которые доходят до кодгена, проходит `llvm-as` (то есть валиден). Ещё 7 тестов ожидаемо падают до кодгена. Так что ниже не «всё сломано», а «вот где хрупко и где переделка окупится».

### 🔴 14.1. Нет `verifyModule` / `verifyFunction`

Сейчас IR валиден (я проверил), **но страховки нет**. Как только начнёшь переделывать, первая же ошибка вида «инструкция после терминатора» или «phi с неправильным входящим блоком» полетит в O2 и `llc` и упадёт там с невнятным сообщением, либо не упадёт и даст UB. Верификатор после генерации модуля, а в Debug ещё после каждой функции, сразу ткнёт в имя функции. Это первое, что надо сделать **до** переделки.

Потенциальные места, где он сработает:
- `Statements::handleReturnStatement`: если `result == nullptr` в не-void функции, то `CreateRet(ptr null)` даже для `i32`. Сейчас семантика не даёт туда попасть, но в переделке легко.
- Любое место, где после `propagatePanic()` / `emitPanic()` забыли `SetInsertPoint`.

### 🔴 14.2. `loadValueIfPointer`: адрес и значение не различаются

Подробно в 6.2. Это корень: `applyCharCast` по пяти местам, `loadValueIfPointer` по десяткам мест, `method_result`-alloca, которая работает «по счастливой случайности», мёртвая ветка `[N x i8]`.

### 🟠 14.3. Результат визита через поле `result`

- Узел, который забыл присвоить `result`, отдаёт **протухший результат предыдущего**. `visit(ModuleMark)` не трогает `result`, как и `visit(ImportNode)`.
- Ради чистого `result` хелперы создают новые `ASTGen` (`Declarations`, `Statements`, `Arrays::emitValue`, `emitElementPtr`, `try`). Их десятки, и у них, например, пустой `currentModuleName`. Сейчас он нигде не используется, но если начнёт использоваться, то сломается в хелперах.

### 🟠 14.4. Пять способов сообщить об ошибке

1. `LogWarning` + `result = nullptr`: без DEBUG **молчит**, и падение случится позже, в другом месте;
2. `throw std::runtime_error("Codegen Error: …")`;
3. `LogError` → `exit(1)`;
4. `return nullptr` из хелпера;
5. `NamedValues.at()` → `std::out_of_range`.

Плюс тихие фолбэки: `getLLVMType` на неизвестном типе даёт `ptr`, `convertValueToType` на неизвестной паре возвращает «как есть», повторное объявление переменной молча игнорируется, неверное число аргументов даёт `nullptr`.

Раз семантика всё проверила, **любая** такая ситуация в кодгене это баг компилятора. Нужен один способ: громко упасть с узлом, строкой и колонкой.

### 🟠 14.5. Два источника правды о «паникующих» функциях + сигнатуры рантайма руками

- `mono.toml`: флаг `panic_context = true` у stdlib-функций;
- `CodeGenContext.cpp:9`: захардкожено `ms_panic_index`, `ms_panic_result`, `ms_str_at`, `ms_map_get`, `ms_map_del`, `ms_map_next`.

Сигнатуры рантайма (`ms_array_reserve`, `ms_str_len`, `ms_map_*`, `GC_malloc*`, `GC_add_roots`, `ms_raise`, `ms_panic`, `strcmp`) объявляются **прямо в местах вызова**, в разных файлах, руками. Если тип разойдётся с D (`int` против `i64`), то это UB, которого не увидит ни верификатор, ни тесты (они могут случайно проходить на x86). И `getOrDeclareFunction` вернёт **первую** объявленную сигнатуру, а вторая, отличающаяся, будет молча проигнорирована.

### 🟡 14.6. TOML и строки без кэша

- `declareFunctionFromTOML` делает `toml::parse(путь)` **при каждом вызове**. Для обычных stdlib-функций это раз на функцию (потом `getFunction` её находит), но для `scat` (`"a" + b`) это **на каждую конкатенацию в программе**: `Expressions.cpp` зовёт `declareFunctionFromTOML("scat", …)` безусловно. Плюс `loader::findTomlPath()` тоже каждый раз.
- `createString` без кэша: одинаковые строки создают новые глобалки (3.2).

### 🟡 14.7. Мёртвый и дублирующийся код

- `Expressions.cpp:188`: ветки `and`/`or`, их перехватывает `visit(BinaryOpNode)`.
- `Expressions.cpp:92` и дальше: в `add/sub/mul/sdiv/srem` блок «если операнд целый → `SIToFP` в **float**». После выравнивания типов выше сюда не дойти. Но если дойдёт, то `i32 + f64` станет `float + f64`, и это невалидный IR (FAdd разных типов). Удалить.
- `loadValueIfPointer`: ветки `[N x i8]` (6.2 ③).
- `visit(BlockNode)`: `isInEntryBlock` не используется.
- `visit(FunctionNode)`: две проверки «дописать ret» (для `entryBlock` и для текущего блока). Достаточно одной: «текущий блок не завершён».
- `getNoneValue()` == `getNullValue()`.
- `Constants.cpp`, `Types.cpp` пустые.
- `handleSimpleAssignment`: `valueType` не используется и вычисляется до проверки на `nullptr` (6.3 ②).
- `visit(VariableAssignNode)`: `llvm::Type* varType;` не инициализирован (6.3 ③).
- Имена вида `%return_load_load`: `loadValueIfPointer(…, "return_load")` добавляет `_load`. Косметика, но говорит о том, что имя передаётся «на всякий случай».

### 🟡 14.8. Лесенка встроенных в `visit(CallNode)`

Схема в 9.1. Каждый новый встроенный это ещё одна ступенька в 200-строчной функции. При этом `len/push/pop/has/del/copy` устроены одинаково: «вычислить первый аргумент, посмотреть его `inferredType`, сделать нужное».

### 🟡 14.9. Области видимости копированием `std::map`

`auto outer = NamedValues; … NamedValues = outer;` в if/else/while/for (×4)/attempt (×3)/функции/конструкторе/`try`. Каждая копия это O(число переменных). Главное же не скорость: каждое новое место, где создаётся область видимости, требует не забыть пару «сохранить/восстановить». Стек областей с `push/pop` делает забывание видимым.

### 🟢 14.10. Мелочи, о которых надо знать

- Конструкторы без проверки стека. Сейчас безопасно, потому что семантика запрещает рекурсивные значения по умолчанию (10.3).
- `ms_array_reserve` вызывается через `CreateCall`, а не `emitCall` (9.2).
- `for k in m` с ключом уже 64 бит читает младшие байты, а это допущение little-endian (11.2).
- `?` с `errorBody` ставит `unreachable`, полагаясь на правило семантики «обработчик обязан выйти» (12.2). Правило есть, но связь нигде не записана.
- `?` всегда аллоцирует новый `result`, даже когда раскладки совпадают (12.2).
- `strcmp` вместо сравнения с учётом длины: O(n) без быстрого выхода, но корректно, пока NUL запрещён.
- `pushLoopContext` пушит условно, а `popLoopContext` попает безусловно (7.4).

### ✅ Что сделано хорошо (не сломай при переделке)

- одна беззнаковая проверка границ ловит и отрицательные индексы (3.3);
- phi в `and`/`or` берёт **последний** блок правой части (8.1);
- `srem` через подмену делителя (8.2);
- `emitFloatToInt`: одна проверка на диапазон и NaN (8.4);
- `for x in a` проверяет, что массив не менялся, и поэтому может не проверять границы (7.3);
- `attempt` сохраняет `stackLimit` (12.3);
- обработчики `attempt` не протекают в функции (5.4 ④);
- значения по умолчанию в полях вычисляются на каждый `P()` (11.1);
- alloca в entry-блоке (6.3);
- `functionAdapter` с отдельным `IRBuilder` (4.7).

---

## Часть 15. План переделки

Каждый шаг маленький и проверяется тестами: `./ms --run_tests` (965 тестов это твоя страховочная сетка).

| # | Шаг | Чинит | Сколько времени | Можно с телефона/урывками |
|---|---|---|---|---|
| 1 | `verifyModule` после генерации + `verifyFunction` в Debug | 14.1 | 15 мин | ✔ |
| 2 | Один способ сообщить о внутренней ошибке, убрать тихие `nullptr` и фолбэки | 14.4 | 1–2 ч, по файлу | ✔ |
| 3 | Таблица рантайм-функций (имя → тип → паникует?) в одном месте + TOML один раз | 14.5, 14.6 | 1–2 ч | ✔ |
| 4 | Кэш строк в `createString` | 14.6 | 15 мин | ✔ |
| 5 | Вычистить мёртвый код | 14.7 | 30 мин | ✔ |
| 6 | **rvalue/lvalue + возврат значения вместо поля `result`** | 14.2, 14.3 | несколько вечеров | ✖ нужна голова целиком |
| 7 | Встроенные в отдельную таблицу | 14.8 | 1 вечер | после 6 |
| 8 | Стек областей видимости | 14.9 | 1 вечер | после 6 |

Как подойти к шагу 6 без большого взрыва:

```
  этап A: завести две новые точки входа рядом со старым визитором
          emitRValue(expr)  — всегда значение
          emitLValue(expr)  — всегда адрес (только Identifier и AccessExpression)
          внутри пока просто зовут старый accept + loadValueIfPointer

  этап B: переводить вызывающих по одному на новые функции
          (Declarations, Return, If-условие, аргументы вызова, операнды …)
          после каждого — тесты

  этап C: когда старый путь никто не зовёт — переписать внутренности
          emitRValue/emitLValue на прямую генерацию по типу узла,
          applyCharCast переезжает в одно место — конец emitRValue

  этап D: удалить loadValueIfPointer и поле result
```

Так у тебя на каждом шаге работающий компилятор и зелёные тесты. Это классический «strangler»-рефакторинг.

---

## Часть 16. Вопросы до переделки

Ответь себе на них **до** шага 6, потому что они меняют форму lvalue-модели и ABI:

1. **Замыкания?** `LambdaNode` сейчас заглушка, а локальные функции не видят переменных внешней. Если замыкания будут, то захваченная переменная должна жить в куче (ячейка), а не в alloca. Значит, `emitLValue` для идентификатора должен уметь вернуть «адрес ячейки в куче». Лучше заложить это в интерфейс сразу.
2. **PanicContext: параметр или thread-local?** Параметр хорош для будущих потоков и уже работает. Thread-local убрал бы скрытый аргумент, `functionAdapter` и ручное добавление контекста в `func<>`-вызовах, но потоки станут сложнее. Решение влияет на ABI всех функций.
3. **Строка как `ptr` на байты с длиной перед ними**: оставляешь? Срезы `s[1:5]` без копирования потребуют `{ptr, len}`, а это ломает ABI с C (`echo(ptr)`) и с D-рантаймом. Чем больше stdlib, тем дороже менять.
4. **Структуры по значению** (внутри массивов или других структур без отдельной аллокации): будут? Если да, то строчка `base = load ptr, elementPtr` в `emitElementPtr` (10.2 ②) перестанет быть верной для таких полей, и lvalue-модель должна знать, «ссылка» это или «встроенный объект».
5. **Проверка стека**: оставить в каждой функции или сделать анализ рекурсии (часть 13)? Не срочно, но если делать, то удобнее, когда кодген уже переписан.

---

*Примеры, на которых снят IR: `01_add`, `02_ctrl` (for/if/while), `03_arr` (массив, push, индекс), `04_struct` (структура, метод), `05_result` (result и `?`), `06_attempt` (attempt/panic). Исходники всех шести есть в тексте выше, их можно скопировать в файлы и повторить.*