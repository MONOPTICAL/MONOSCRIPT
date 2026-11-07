// expect-output: hi
// expect-output: 1.5
// expect-output: true
// expect-output: x
[void]say(string: s)
|   echo(s)

[void]run(func: f(string: s) -> void, string: v)
|   f(v)

[f64]half(f64: x)
|   return x / 2.0

[f64]onF(func: f(f64: x) -> f64)
|   return f(3.0)

[i1]yes(char: c)
|   return c == 'x'

[i1]onC(func: f(char: c) -> i1)
|   return f('x')

[char]first(string: s)
|   return s[0]

[char]onS(func: f(string: s) -> char)
|   return f("xy")

[i32]main()
|   run(say, "hi")
|   echo(cstr(onF(half)))
|   echo(cstr(onC(yes)))
|   echo(cstr(onS(first)))
|   return 0
