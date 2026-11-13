// expect-output: 2
// expect-output: 10
[namespace] math
|   [i32]f(i32: x)
|   |   return x + 1

[i32]f(i32: x)
|   return x * 10

[i32]main()
|   echo(toString_int(math.f(1)))
|   echo(toString_int(f(1)))
|   return 0
