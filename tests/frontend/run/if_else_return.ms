// expect-output: 1
// expect-output: 2
[i32]f(i32: a)
|   if a > 0
|   |   return 1
|   else
|   |   return 2

[i32]main()
|   echo(toString_int(f(5)))
|   echo(toString_int(f(-5)))
|   return 0
