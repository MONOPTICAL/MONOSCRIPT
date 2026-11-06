// expect-output: 5
// expect-output: x
[i32]f()
|   i32 x = 5
|   array<i32> a = [1]
|   return x

[string]g()
|   string s = "x"
|   array<i32> a = [1]
|   return s

[i32]main()
|   echo(toString_int(f()))
|   echo(g())
|   return 0
