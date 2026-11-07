// expect-error: 5: @strict
[i32]main() @strict
|   i64 e = 4
|   i32 s = 2
|   for i in range(s, e)
|   |   echo("x")
|   return 0
