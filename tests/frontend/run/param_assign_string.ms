// expect-output: x
// expect-output: a
[string]f(string: s)
|   s = "x"
|   return s

[i32]main()
|   string v = "a"
|   echo(f(v))
|   echo(v)
|   return 0
