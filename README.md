<div align="center">
  <a href="https://github.com/MONOPTICAL/MONOSCRIPT">
    <img src="https://github.com/MONOPTICAL/MONOSCRIPT/blob/main/MONOSCRIPT.png?raw=true" alt="Logo">
  </a>

<h3 align="center">MONOSCRIPT</h3>

  <p align="center">
    Convenient scripting language for creating plugins for MONOPTICAL
    <br />
  </p>
  <hr>
</div>

**MONOSCRIPT** is an experimental programming language with both static and dynamic typing. It features a concise syntax inspired by **Python**, **C**, and functional ideas from **Elixir** and **F#**. Designed as an **embedded scripting language**, it emphasizes:

- safe execution,
    
- modularity,
    
- extensibility,
    
- ease of learning and code readability.
    

It was created to be used in closed environments, such as **security plugins**, **scanning systems**, **data analytics**, **automation**, and as part of the **MONOPTICAL** project.

## 🧭 Paradigms

MONOSCRIPT combines several programming paradigms:

|Paradigm|Description|
|---|---|
|Imperative|Traditional constructs like `if`, `for`, `while`, mutability, and step-by-step execution.|
|Functional|Lambdas, pure functions, pipe operators.|
|Modular|Organized via `use`, modules, and namespaces.|
|Structural|Custom `struct`s with methods and encapsulation support.|

> About OOP The language is **not object-oriented** in the classic sense (no class inheritance), but **structs with methods** offer similar code organization.

## 🔠 Data Types

---

**MONOSCRIPT** features a compact and expressive type system aimed at both strict static checks and flexible dynamic usage. It avoids common runtime errors like null dereferencing and type mismatches.

### 📃 Primitive Types

- `i1`, `i8`, `i16`, `i32`, `i64` — integers with varying widths
    
- `float` — floating-point numbers
    
- `string` — text values
    
- `void` — absence of value (used for functions)
    

### 📦 Containers and Structs

- `array<T>` — arrays of fixed or dynamic size
    
- `map<K, V>` — key-value mappings (dictionaries)
    
- `struct` — user-defined structures with fields and methods
    

### ⚙️ Typing

MONOSCRIPT's type system adapts to the use case:

|Characteristic|Details|
|---|---|
|Static|`type name = value` declares explicit type.|
|Dynamic|`name ^= value` infers the type automatically.|
|Hybrid|Both styles can be used within the same program.|
|Null-safe|Variables are non-null by default; checked explicitly via `?x`.|

> This safety reduces bugs and makes code behavior more predictable while remaining concise.

## 📋 Syntax

### 📌 Key Principles

- **Readability over brevity**.
    
- **Blocks start with `|`** — makes structure visually clear.
    
- **Return types in square brackets** before function names:
    

```go
[i32]sum(i32: a, i32: b)
```

### 🧱 Operators

|Operator|Meaning|
|---|---|
|`=`|Assignment (static type)|
|`^=`|Assignment with inferred type|
|`+ - * / %`|Arithmetic|
|`== != > <`|Comparison|
|`and or`|Logical operations|
|`!expr`|Negation|
|`:`|Start of multi-line lambda|
|`->`|One-liner lambda body or Casting|

### ⚖️ Control Structures

- `if`, `else if`, `else`
    
- `for i in range(...)`
    
- `while ...`
    
- `break`, `continue`
    

### 📍 Declarations

- `const` — constant with explicit type
    
- `final` — constant with inferred type
    
- `use |> module` — module import
    
- `struct`, `return`, `null`
    

### 🧪 Functions & Lambdas

**Named functions:**

```go
[i32]sum(i32: a, i32: b)
|   return a + b
```

**One-liner lambda:**

```go
[i8](i8: x) -> x * x
```

**Block lambda:**

```go
[array<i8>](array<i8>: a) :
|   for i in range(a)
|   |   a[i] = a[i] * 2
```

### 🔗 Pipes (`|>`) — Functional Chains

```go
x ^= [1,2,3]
    |> map(x, [i32](i32: a) -> a + 1)
    |> filter(x, [i32](i32: a) -> a > 2)
```

- Each step gets the output of the previous.
    
- Enables readable, declarative chaining.
    

### 🧰 Structs & Methods

```go
[struct]User
|  map<string,string> info
|  [void]__init__(string: name, string: id)
|  |  info = {"name": name, "id": id}
|  [string]getName()
|  |  return info["name"]
```

- Methods are regular functions inside structs.
    
- Encapsulation via internal fields.
    
- No inheritance.
    

### 📥 Imports (`use`)

```cpp
use
|> http_server
|> math -> sqrt : sqrtAlias
```
**Importing modules and functions** is done using the `use` keyword and pipe chains.

- The module `http_server` is imported, then from the module `math` the function `sqrt` under the alias `sqrtAlias`.
- You can import multiple modules and functions in one chain.

### 🏷️ Labels

```go
[i32]main() @entry @strict
|   echo("Hello World!")
```
**Labels** allow you to mark functions, variables, or structures with special attributes that affect their behavior during compilation or execution.

- Labels are specified after the declaration using `@`.
- Multiple labels can be used simultaneously.


### 🪜 Security

- **Null safety** — checked explicitly with `?x`.
    
- **Sandboxing** — interpreter runs in isolated environments.
    

### ⚖️ Embeddability & Extensibility

- MONOSCRIPT is embeddable in other apps.
    
- Core interpreter written in **C++**, **D** and **Zig**.
    
- Easily extended with native modules.
    
- Module system via `use` is supported.
    

## 📄 License

MONOSCRIPT is an open-source project under the GNU LGPL v2.1:

- You can use it freely.
    
- Engine changes must remain open.
    
- Embedding does not require disclosing your source.
    

## ❤️ Conclusion

**MONOSCRIPT is not trying to compete with giants. It's about solving real problems in a simple, secure, and enjoyable way.**

Perfect for:

- scripting and logic in security systems,
    
- DevOps/Kubernetes automation,
    
- safe and readable mini-programs,
    
- educational and embedded projects.
    

```rust
[i64]fibonacci(i64: n) @strict @pure
|   if (n == 0)
|   |   return 0
|   else if (n == 1)
|   |   return 1
|   return fibonacci(n - 1) + fibonacci(n - 2)

[i32]main()
|   echo(toString_long(fibonacci(47))) // 2971215073
|   return 0
```
