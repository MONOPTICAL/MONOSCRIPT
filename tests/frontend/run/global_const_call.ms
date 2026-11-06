// expect-output: 7
[i32]f()
|   return 7

const i32 g = f()

[i32]main()
|   echo(toString_int(g))
|   return 0
