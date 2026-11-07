<div align="center">
  <a href="https://github.com/MONOPTICAL/MONOSCRIPT">
    <img src="https://github.com/MONOPTICAL/MONOSCRIPT/blob/main/MONOSCRIPT.png?raw=true" alt="Logo">
  </a>

<h3 align="center">MONOSCRIPT</h3>

  <p align="center">
    Convenient language for creating plugins for MONOPTICAL
    <br />
  </p>
  <hr>
</div>

**MONOSCRIPT** is an experimental, statically typed programming language with type inference, compiled through **LLVM**. It feels like a scripting language, runs instantly through a JIT, and can also be built into a native executable.

```go
[i64]fibonacci(i64: n) @strict @pure
|   if n < 2
|   |   return n
|   return fibonacci(n - 1) + fibonacci(n - 2)

[i32]main()
|   echo("fib(30) = " + fibonacci(30))
|   return 0
```

> 🚧 **Early development.** Numbers, strings, functions, loops, modules, arrays, maps and structs work today, through the JIT and in native executables. See [Current State](#-current-state).

## 💡 Why MONOSCRIPT?

MONOPTICAL needs plugins: small programs for security checks, scanning, data analytics and automation. They have to be quick to write, and they must not fail in surprising ways.

Scripting languages are quick to write, but type errors only show up when the code is already running. Systems languages catch those errors, but they are too heavy for a small plugin. MONOSCRIPT aims for the middle:

|Goal|How|
|---|---|
|Quick to write|Short syntax, type inference, run a file with one command|
|Hard to break|Static types, no silent narrowing, errors at compile time|
|Strict where it matters|Turn on `@strict` and `@pure` per function, not for the whole project|
|Fast|Native code through LLVM instead of an interpreter|
|Easy to extend|Add native functions in **D** with one entry in a TOML file|

## ✨ What Makes It Different

### 🏷️ Strictness Per Function

You choose how strict each function is. Quick code stays quick, and critical code gets stronger guarantees:

```go
[i32]quick()
|   i8 small = 5
|   i32 wide = small          // widens automatically
|   return wide

[i32]careful() @strict
|   i8 small = 5
|   i32 wide = small->i32     // @strict requires an explicit cast
|   return wide

[i32]main()
|   echo("" + quick() + careful())
|   return 0
```

### 🧼 Purity That Is Actually Checked

`@pure` is a promise the compiler enforces. A pure function cannot call impure functions, read globals or modify the objects passed to it:

```go
[string]label(i32: score) @pure
|   return "score: " + score    // OK

[void]report(i32: score) @pure
|   echo(label(score))          // compile error: echo is impure

[i32]main()
|   return 0
```

### 🔢 No Silent Narrowing

Widening is automatic. Anything that can lose data needs an explicit cast:

```go
[i32]main()
|   i32 big = 300
|   i64 wide = big          // OK
|   i8 small = big->i8      // OK: you asked for it
|   i8 oops = big           // compile error: use explicit cast ->i8
|   return 0
```

### 🧱 Visible Structure

Blocks start with `|`, so nesting is visible at a glance and does not depend on invisible whitespace:

```go
[i32]main()
|   for i in range(10)
|   |   if i % 2 == 0
|   |   |   echo("even: " + i)
|   return 0
```

### 🔌 Explicit Modules

Import a whole module, a single function or a function under an alias. Functions can stay `@private`, and replacing an imported function has to be intentional:

```go
use
|> http_server
|> math -> sqrt
|> math -> pow : power

[i32]helper() @overwrite    // without @overwrite, a name clash is a compile error
|   return 100
```

## 🧭 Paradigms

|Paradigm|Description|
|---|---|
|Imperative|`if`, `for`, `while`, mutable variables, step-by-step execution.|
|Modular|`use`, `@private` and `@overwrite`.|
|Structural|Custom `struct`s, no class inheritance.|
|Functional|Pure functions with `@pure`. Lambdas and pipes are planned.|

## 🔠 Data Types

### 📃 Primitive Types

- `i1` — boolean, `true` and `false`
- `i8`, `i16`, `i32`, `i64` — signed integers
- `float` — 32-bit floating-point numbers
- `f64` — 64-bit floating-point numbers
- `string` — immutable UTF-8 bytes with a stored length; embedded NUL bytes are rejected
- `char` — one byte: `'a'`, `'\\n'`. `s[i]`, `for c in s` and `len(s)` work in bytes
- `void` — absence of value (used for functions)

### 📦 Containers and Structs

- `array<T>` — arrays
- `map<K, V>` — key-value mappings
- `struct` — user-defined structures

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

### ⚙️ Typing

|Form|Example|
|---|---|
|Explicit type|`i32 count = 0`|
|Inferred type|`name ^= "value"`|
|Constant|`const i32 limit = 10`, `final limit ^= 10`|
|Assigned once, later|`final i32 limit`, then `limit = 10` on every path before it is read|
|Cast|`value->i8`|

## 📋 Syntax

### 🧪 Functions

The return type goes in square brackets, and the parameter type goes before its name:

```go
[i32]sum(i32: a, i32: b)
|   return a + b

[i32]main()
|   echo("1 + 2 = " + sum(1, 2))
|   return 0
```

### 🧱 Operators

|Operator|Meaning|
|---|---|
|`=`|Assignment|
|`^=`|Declaration with inferred type|
|`+ - * / %`|Arithmetic, `+` also joins strings|
|`== != > < >= <=`|Comparison of numbers and strings|
|`and or !`|Logical operations|
|`->`|Cast|

### ⚖️ Control Structures

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

### 🏷️ Labels

Labels go after the function declaration:

|Label|Effect|
|---|---|
|`@entry`|Marks the entry point instead of `main`|
|`@strict`|No implicit casts and no type inference|
|`@pure`|No side effects, checked by the compiler|
|`@private`|Hidden from imports|
|`@overwrite`|Replaces an imported function on purpose|

### 🧩 Generic Functions

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

### 🔁 Passing Functions

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

A function can only be passed as an argument, not stored in a variable. `push(a, x)` and `pop(a)` grow and shrink an array.

### ⚠️ Errors Without Crashes

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

Postfix `r?` unwraps a successful result. On error it immediately returns the original error code and message from the enclosing function, which must return `result<U>`; `U` may differ from the unwrapped type. Prefix `?r` remains a boolean success check. The operand is evaluated once; postfix `?` binds before arithmetic and works with `result<void>` as a standalone statement.

```go
[result<i32>]sumNumbers(string: a, string: b)
|   i32 x = toInt(a)?
|   i32 y = toInt(b)?
|   return ok(x + y)
```

Use `r?->i64` for a cast after unwrapping. A nested result can be unwrapped with `r??`. Mutable values borrowed from arguments remain subject to `@pure` checks.

`toInt(s)` returns `result<i32>`: it accepts decimal digits with an optional leading `+` or `-`. Empty input, whitespace, trailing characters and values outside the `i32` range return errors (code 1 for syntax, 2 for range). Check `.ok` before reading `.value`.

Reading `r.value` from an error stops the program with the original message. A result that is never read is a compile error, write `_ ^= save()` to ignore one on purpose. Use `result<void>` and `return ok()` when there is no value. A native function in D returns the same layout with `resultOk`, `resultErr`, `resultVoidOk` and `resultVoidErr` from `mono/std/core/abi.d`.

Native string returns must use `newString(length)`, `stringFromBytes(data, length)` or `stringFromCString(text)` from the same ABI module. Strings still pass a C-compatible data pointer, but now carry a `u64` byte length immediately before it. Do not return raw C/D literals or `malloc` buffers as language strings. `resultErr` copies native error messages into this format. Rebuild native extensions and executables against the updated runtime.

### 🧰 Structs

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

## 📍 Current State

|Feature|Status|
|---|---|
|Numbers, strings, operators and casts|✅ Working|
|`if`, `while`, `for i in range(n)`|✅ Working|
|Functions, recursion, calling before declaration|✅ Working|
|Modules, `@strict`, `@pure`, `@entry`|✅ Working|
|JIT and native executables|✅ Working|
|Arrays: literals, nesting, `a[i]`, `len`, `for x in array`, bounds checks|✅ Working|
|Maps: `m[k]`, `has`, `del`, `len`, `for k in map`|✅ Working|
|Structs: `P()`, fields, nesting, arrays of structs|✅ Working|
|`copy` for arrays, maps and structs|✅ Working|
|Garbage collection (Boehm)|✅ Working|
|Runtime errors with line numbers|✅ Working|
|Struct methods|✅ Working|
|Lambdas and pipes (`\|>`)|📅 Planned|
|Errors as values: `result<T>`, `ok()`, `err()`, `?r`|✅ Working|
|Generic functions and passing functions by name|✅ Working|
|Null safety|📅 Planned|
|Embedding and sandboxing|📅 Planned|

## 🚀 Getting Started

You will need CMake 3.24+, a C++20 compiler, LLVM, `dmd`, `gcc` and `libgc-dev`.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
```

```bash
cmake --build build
```

Run a program with the JIT:

```bash
./ms run hello.ms
```

Build a native executable:

```bash
./ms --compile hello hello.ms
```

See which compiler stage takes the time:

```bash
./ms run hello.ms --timings
```

Run the test suite:

```bash
./ms --run_tests
```

## 📄 License

MONOSCRIPT is an open-source project under the GNU LGPL v2.1:

- You can use it freely.

- Engine changes must remain open.

- Embedding does not require disclosing your source.

## ❤️ Conclusion

**MONOSCRIPT is not trying to compete with giants. It's about solving real problems in a simple, secure, and enjoyable way.**

Made for:

- scripting and logic in security systems,

- scanning and data analytics,

- DevOps and automation,

- small, safe and readable programs.
