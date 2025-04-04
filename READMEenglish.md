# MONOSCRIPT Language

MONOSCRIPT is a lightweight, human-readable programming language inspired by Python and C-style syntax. It is designed for writing secure, modular scripts with a focus on simplicity and plugin integration. MONOSCRIPT is especially useful in environments like Kubernetes for writing custom security plugins.

---

## 📦 Features

### ✅ Clean Function Syntax
```htpy
[i32]sum(i32: a, i32: b)
|   return a + b
```
- Type annotations use brackets.
- Pipe (`|`) represents indentation.

---

### ✅ Nested Functions
```htpy
[void]main()
|   [i32]sum(i32: a, b, c)
|   |   return a + b + c
```
- You can define functions inside functions.

---

### ✅ Multiple Arguments of One Type
```htpy
[i32]sum(i32: a, b, c)
```
- Cleaner syntax for same-type parameters.

---

### ✅ Prefix/Unary Operators
```htpy
negTotal ^= -total
confirmed ^= !isSafe
```
- Supports `-`, `!` as standalone unary operators.

---

### ✅ Control Flow
```htpy
if x > 0 and y < 5
|   return x
else
|   return y
```
- Fully supports `if`, `else`, `for`, `while`, `return`, etc.

---

### ✅ Loops with Range
```htpy
for i in range(0, len(pods))
|   echo(pods[i])
```
- Python-style `for in range(...)` loops.

---

### ✅ Arrays and Indexing
```htpy
value ^= pods[i]
```
- Full support for bracket indexing.

---

### ✅ Function Calls & Nesting
```htpy
level ^= severityLevel(scanPod(pod))
```
- Any depth of function nesting is supported.

---

### ✅ String Operations
```htpy
echo("Scan result: " + podName + ": " + result)
```
- Strings + variables can be concatenated using `+`.

---

### ✅ Boolean Literals & Logic
```htpy
flag ^= false
result ^= !flag
```
- Supports `true`, `false`, `null`, `!`, `and`, `or`.

---

### ✅ Comments
```htpy
// This is a comment
```
- Line comments use `//`, ignored by lexer.

---

### ✅ Assignment & Operators
```htpy
value ^= 5
value ^= functionCall()
```
- `^=` is used for smart assignment (declaration+assignment).

---

## 🚀 Use Cases
- Writing plugin logic for MONOPTICAL (Kubernetes security)
- Scripting small internal tools
- Teaching programming with readable syntax

---

## 🛠 Example
```htpy
[void]main()
|   [i32]sum(i32: a, b, c)
|   |   return a + b + c
|
|   total ^= sum(10, 20, 30)
|   echo("Total is: " + total)
```

---

Made with ❤️ by the MONOPTICAL Team

