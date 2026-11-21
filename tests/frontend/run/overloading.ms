// expect-output: int 5
// expect-output: str a
// expect-output: long 7
// expect-output: f64 1.5
// expect-output: two 3
// expect-output: int 3
// expect-output: int -4
// expect-output: int 200
// expect-output: long 4000000000
// expect-output: int 9
// expect-output: 212
[string]show(i32: x)
|   return "int " + x

[string]show(string: s)
|   return "str " + s

[string]show(i64: x)
|   return "long " + x

[string]show(f64: x)
|   return "f64 " + x

[string]show(i32: a, i32: b)
|   return "two " + (a + b)

[namespace] m
|   [i32]f(i32: x)
|   |   return 1
|   [i32]f(string: s)
|   |   return 2
|   [i32]both()
|   |   return f(1) * 10 + f("a")

[i32]ap(func: f(i32: x) -> string, i32: v)
|   echo(f(v))
|   return 0

[i32]main()
|   echo(show(5))
|   echo(show("a"))
|   i64 w = 7
|   echo(show(w))
|   echo(show(1.5))
|   echo(show(1, 2))
|   i8 small = 3
|   echo(show(small))
|   echo(show(-4))
|   u8 b = 200
|   echo(show(b))
|   u32 big = 4000000000
|   echo(show(big))
|   _ ^= ap([string](i32: x) -> show(x), 9)
|   echo(cstr(m.f("x") * 100 + m.both()))
|   return 0
