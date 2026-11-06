// expect-output: 7
// expect-output: 2
[i32]f(i32: x, array<string>: a)
|   echo(toString_int(x))
|   return len(a)

[i32]main()
|   echo(toString_int(f(7, ["a","b"])))
|   return 0
