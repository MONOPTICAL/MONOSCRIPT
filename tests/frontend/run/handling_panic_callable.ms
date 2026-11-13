// expect-output: division by zero
// expect-output: strings cannot contain a NUL byte
// expect-output: 1
// expect-output: division by zero
// expect-output: division by zero
[i32]danger(i32: n)
|   return 10 / n
[i32]invoke(func: f(i32: n) -> i32)
|   return f(0)
[string]native(func: f(char: c) -> string)
|   return f(0->char)
[i32]plainNative(func: f(string: a, string: b) -> i32)
|   return f("abc", "b")
[struct]Broken
|   i32 n = danger(0)
[struct]Box
|   i32 n = 1
|   [i32]bad()
|   |   return danger(0)
[i32]main()
|   attempt
|   |   n ^= invoke(danger)
|   panic info
|   |   echo(info.message)
|   attempt
|   |   echo(native(toString_char))
|   panic info
|   |   echo(info.message)
|   echo("" + plainNative(strfind))
|   attempt
|   |   Broken b = Broken()
|   panic info
|   |   echo(info.message)
|   Box b = Box()
|   attempt
|   |   n ^= b.bad()
|   panic info
|   |   echo(info.message)
|   return 0
