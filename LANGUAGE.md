# The MONOSCRIPT Language

The full tour: types, syntax, error handling and the native ABI. For the short pitch, see the [README](README.md).

## Paradigms

|Paradigm|Description|
|---|---|
|Imperative|`if`, `for`, `while`, mutable variables, step-by-step execution.|
|Modular|`use`, `@private` and `@overwrite`.|
|Structural|Custom `struct`s, no class inheritance.|
|Functional|Pure functions with `@pure`, lambdas with closures. Pipes are planned.|

## Data Types

### Primitive Types

- `i1` — boolean, `true` and `false`
- `i8`, `i16`, `i32`, `i64` — signed integers
- `u8`, `u16`, `u32`, `u64` — unsigned integers. Division, `%`, comparison and `shr` work without sign
- `float` — 32-bit floating-point numbers
- `f64` — 64-bit floating-point numbers
- `string` — immutable UTF-8 bytes with a stored length; embedded NUL bytes are rejected
- `char` — one byte: `'a'`, `'\\n'`. `s[i]`, `for c in s` and `len(s)` work in bytes
- `void` — absence of value (used for functions)

### Containers and Structs

- `array<T>` — arrays
- `map<K, V>` — key-value mappings
- `struct` — user-defined structures, also generic: `[struct]Stack<T>`

A generic struct gets its type arguments from where the value goes: `Stack<i32> s = Stack()`, a `return Stack()` from a function returning `Stack<i32>`, or a field default. A field of type `T` may have no value and starts as the zero value of its type: `0`, `""`, an empty collection or a new struct. Each `Stack<i32>` is a separate copy, like a generic function; an operation `T` does not support is reported when that copy is checked. A generic function cannot take `Stack<T>` yet, use a method instead.

An integer converts implicitly only when every value fits: `u8` into `u16`, `i16` or `i32`, `i8` into `i64`. A signed value never becomes unsigned implicitly, and mixing `i32` with `u32` in one expression is an error, so write `->u32`. An explicit integer cast keeps the low bits: `300->u8` is 44 and `(-1)->u8` is 255. A float that does not fit the target integer type panics. An unsigned literal is at most the `i64` maximum; build larger `u64` values with an expression, such as `bnot 0->u64`.

Like in Python, numbers are copied, while objects are passed by reference. Use `copy(x)` when you need a separate shallow copy. In `@pure` functions, copies that still share nested mutable objects with an argument are rejected. Returning or wrapping borrowed mutable objects is also rejected; this check is conservative. Objects live on a garbage-collected heap, and a reference is never null.

```go
[i32]main()
|   array<i32> scores = [10, 20, 30]
|   scores[0] = 5
|   for score in scores
|   |   echo("score: " + score)
|
|   map<string, i32> ports = {"ssh": 22}
|   ports["http"] = 80
|   if has(ports, "ssh")
|   |   del(ports, "ssh")
|   for name in ports                // keys come in insertion order
|   |   echo(name + " = " + ports[name])
|
|   return len(scores) + len(ports)
```

Map keys are integers or strings. An index out of bounds, a missing map key, an integer division by zero, or a cast from `float` that does not fit stops the program with a message, the line number and exit code 101:

```
Runtime Error: index 3 out of bounds (len 3) at line 5
```

Integer addition, subtraction and multiplication wrap around on overflow, so `2147483647 + 1` in `i32` is `-2147483648`. The one integer operation that stops the program is dividing the smallest value of a type by `-1`.

### Typing

|Form|Example|
|---|---|
|Explicit type|`i32 count = 0`|
|Inferred type|`name ^= "value"`|
|Constant|`const i32 limit = 10`, `final limit ^= 10`|
|Assigned once, later|`final i32 limit`, then `limit = 10` on every path before it is read|
|Cast|`value->i8`|

## Syntax

### Functions

The return type goes in square brackets, and the parameter type goes before its name:

```go
[i32]sum(i32: a, i32: b)
|   return a + b

[i32]main()
|   echo("1 + 2 = " + sum(1, 2))
|   return 0
```

### Operators

|Operator|Meaning|
|---|---|
|`=`|Assignment|
|`^=`|Declaration with inferred type|
|`+ - * / %`|Arithmetic, `+` also joins strings|
|`**`|Power, binds tighter than `*` and from the right: `2 ** 3 ** 2` is `2 ** 9`. Integer power with a negative exponent panics|
|`band bor xor shl shr bnot`|Bitwise operations on integers, written as words because `\|` marks indentation. They bind tighter than comparison: `x band 1 == 0` is `(x band 1) == 0`. `shr` keeps the sign of a signed value and fills an unsigned one with zeros; shifting by a negative amount or by the type's width or more panics|
|`== != > < >= <=`|Comparison of numbers and strings|
|`and or !`|Logical operations|
|`->`|Cast|

### Control Structures

- `if`, `else if`, `else`
- `for i in range(n)`, `for i in range(a, b)`, `for x in array`, `for key in map`
- `while ...`
- `break`, `continue`

A single simple statement can follow `:` on the same line. `else` and `else if` stay on their own lines; inline and indented branches can be mixed:

```go
[i32]sign(i32: n)
|   if n < 0: return -1
|   else if n == 0: return 0
|   else: return 1
```

An inline branch contains exactly one statement. Nested control blocks use the indented form.

### Labels

Labels go after the function declaration:

|Label|Effect|
|---|---|
|`@entry`|Marks the entry point instead of `main`|
|`@strict`|No implicit casts and no type inference|
|`@pure`|No side effects, checked by the compiler|
|`@private`|Hidden from imports|
|`@overwrite`|Replaces an imported function on purpose|

### Generic Functions

A function can work with any type. The types are worked out from the arguments, and the compiler checks and generates a separate copy for each set of types:

```go
[T]first<T>(array<T>: items)
|   return items[0]

[array<T>]pair<T>(T: a, T: b)
|   return [a, b]

[i32]main()
|   echo(first(["a", "b"]))     // T is string
|   echo("" + first([1, 2]))    // T is i32
|   array<f64> p = pair(0.5, 1.5)
|   return 0
```

### Passing Functions

A function can be passed by name to a parameter of type `func`. Together with generic functions this is enough to write `filter`, `map` or `sort` in the language itself:

```go
[array<T>]filter<T>(array<T>: items, func: keep(T: x) -> i1)
|   array<T> out = []
|   for x in items
|   |   if keep(x)
|   |   |   push(out, x)
|   return out

[i1]isEven(i32: x)
|   return x % 2 == 0

[i32]main()
|   array<i32> even = filter([1, 2, 3, 4], isEven)
|   return len(even)
```

A named function can only be passed as an argument. `push(a, x)` and `pop(a)` grow and shrink an array.

### Lambdas

A lambda is written like a function header without a name, with `->` and an expression or `:` and a block:

```go
[i32]main()
|   i32 k = 10
|   array<i32> big = filter([1, 20, 30], [i1](i32: x) -> x > k)
|   func<i32, i32> twice = [i32](i32: x) -> x * 2
|   f ^= [i32](i32: x):
|   |   echo("got " + x)
|   |   return x + 1
|   return twice(f(1))
```

A lambda captures the local variables it sees by value, at the moment it is created: changing `k` later does not change the lambda. A captured variable cannot be assigned inside the lambda, but the object it refers to can change, so `push(log, x)` on a captured array works. A lambda can be kept in a local variable declared as `func<...>` or with `^=`.

### Overloading

Functions with the same name and different parameters are separate functions. A call picks the one whose parameters fit the arguments best: an exact type, then the nearest lossless widening, then an integer to a float. An integer literal fits `i32` best and a decimal literal fits `f64`. Two equally good candidates make the call ambiguous; add `->` to choose. An overloaded function cannot be passed by name; wrap the one you need in a lambda. Generic functions and `main` cannot be overloaded.

```go
[string]show(i32: x)
|   return "int " + x

[string]show(string: s)
|   return "str " + s

[i32]main()
|   echo(show(5))       // int 5
|   echo(show("a"))     // str a
|   return 0
```

### Errors Without Crashes

Something that can fail returns `result<T>`. The caller checks it, and the program keeps running:

```go
[result<i32>]parseDigit(string: s)
|   if len(s) != 1
|   |   return err(1, "expected one digit")
|   char c = s[0]
|   if c < '0' or c > '9'
|   |   return err(2, "not a digit: " + s)
|   return ok(c->i32 - '0'->i32)

[i32]main()
|   result<i32> r = parseDigit("x")
|   if ?r                        // the same as r.ok
|   |   echo("digit " + r.value)
|   else
|   |   echo("error " + r.code + ": " + r.error)
|   return 0
```

Postfix `r?` unwraps a successful result. Without a local handler, an error immediately returns the original error code and message from the enclosing function, which must return `result<U>`; `U` may differ from the unwrapped type. Prefix `?r` remains a boolean success check. The operand is evaluated once; postfix `?` binds before arithmetic and works with `result<void>` as a standalone statement.

```go
[result<i32>]sumNumbers(string: a, string: b)
|   i32 x = toInt(a)?
|   i32 y = toInt(b)?
|   return ok(x + y)
```

Use `r?->i64` for a cast after unwrapping. A nested result can be unwrapped with `r??`. Mutable values borrowed from arguments remain subject to `@pure` checks.

`toInt(s)` returns `result<i32>`: it accepts decimal digits with an optional leading `+` or `-`. Empty input, whitespace, trailing characters and values outside the `i32` range return errors (code 1 for syntax, 2 for range). Check `.ok` before reading `.value`.

Reading `r.value` from an error raises a runtime panic with the original message; it exits with code 101 if no panic handler catches it. A result that is never read is a compile error, write `_ ^= save()` to ignore one on purpose. Use `result<void>` and `return ok()` when there is no value. A native function in D returns the same layout with `resultOk`, `resultErr`, `resultVoidOk` and `resultVoidErr` from `mono/std/core/abi.d`.

Local handling also works in ordinary functions such as `[i32]main()`:

```ms
[i32]main()
|   port ^= toInt("invalid")? else 8080
|   n ^= toInt("42")? else: echo(error.error); return error.code
|
|   attempt
|   |   value ^= toInt("7")?
|   |   array<i32> rows = [value]
|   |   echo("" + rows[9])
|   else error
|   |   echo(error.error)
|   panic info
|   |   echo(info.message + " at line " + info.line)
|   return 0
```

`? else expression` evaluates the result once and evaluates the replacement only on error. The replacement must fit the unwrapped type; use parentheses to delimit it inside a larger expression. `result<void>` has no replacement value. `? else: statements` binds the original error as `error` (`result<void>`, read-only `.ok`, `.code`, `.error`) and must end in `return`, `break` or `continue`; semicolons separate statements in this inline handler. Its binding is scoped to the handler. The action form consumes the remainder of the line and should be used as the final expression of a statement.

Within an `attempt`, bare `?` sends an error to the nearest enclosing `else name` handler in the **same function**. Called functions still propagate their results through their own return values. An attempt may have an error handler, a panic handler, or both; duplicate handlers are rejected. Body variables are not visible in its handlers or after the attempt. After a handler finishes, execution continues after the entire attempt. A failure inside a handler targets an outer handler, never another handler of the same attempt.

`panic info` catches controlled runtime failures, including those in deeper calls: integer division by zero/overflow, invalid numeric casts, array/string bounds, empty `pop`, missing map keys, map mutation during iteration, stack overflow, erroneous `.value` access, and checked standard-library string operations. It binds an immutable `panic_info` snapshot with `.message` and `.line` (0 if unknown). `? else` handles returned result errors only: a panic while evaluating the operand or replacement still goes to a surrounding panic handler. Side effects are not rolled back. OOM and arbitrary native crashes remain fatal. There is no `defer` or automatic resource cleanup yet; catching a panic does not close files, release locks, or undo a partial response.

The compiler passes an explicit hidden `MsPanicContext*` through language calls, including methods, constructors and `func<>` calls; ordinary values keep their existing layouts. After a failed call, the compiler checks the channel before using its return value. Each program invocation owns its context; there is no global error flag. Language function symbols are internal to the compiled module, so their private calling convention cannot override C runtime functions with the same name.

Native functions opt in with `panic_context = true` in `mono.toml` and accept a final `MsPanicContext*` argument, in addition to their declared language arguments. They report a controlled failure with `ms_raise(message, line, context)` and **return immediately**. They must also propagate failures from their own helpers before using any placeholder return value. `ms_raise` copies the message into a language string. A native function without this contract remains an ordinary C call; calling `ms_panic` still exits. Rebuild executables and native libraries after this calling-convention change; an old binary is not compatible with the updated checked standard-library entry points.

Native string returns must use `newString(length)`, `stringFromBytes(data, length)` or `stringFromCString(text)` from the same ABI module. Strings still pass a C-compatible data pointer, but now carry a `u64` byte length immediately before it. Do not return raw C/D literals or `malloc` buffers as language strings. `resultErr` copies native error messages into this format. Rebuild native extensions and executables against the updated runtime.

### Structs

```go
[struct]User
|   i32 age = 18
|   string name = "anon"

|   [void]birthday()
|   |   self.age = self.age + 1
|
|   [string]describe() @pure
|   |   return self.name + " is " + self.age

[i32]main()
|   User user = User()
|   user.birthday()
|   echo(user.describe())
|   return 0
```

Inside a method, `self` is the object. A constant object can only call `@pure` methods, and a `@pure` method cannot change `self`.

### Enums

```go
[enum]Color
|   Red
|   Green
|   Blue = 10        // the next member is 11

[i32]main()
|   Color c = Color.Blue
|   if c == Color.Blue
|   |   echo(cstr(c))         // 10
|   return (1->Color)->i32    // 1
```

An enum is its own type stored as `i32`. Its values only support `==` and `!=`, cannot be assigned or mixed with numbers implicitly, and convert with `->` to and from integers. `cstr` prints the number.


### Modules

Import a whole module, a single function or a function under an alias. Functions can stay `@private`, and replacing an imported function has to be intentional:

```go
use
|> http_server
|> math -> sqrt
|> math -> pow : power

[i32]helper() @overwrite    // without @overwrite, a name clash is a compile error
|   return 100
```
