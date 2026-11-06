// expect-output: 5
// expect-output: 7
[i32]main()
|   i32 x = 0
|   if x == 0
|   |   x = 5
|   echo(toString_int(x))
|   while x < 7
|   |   x = x + 1
|   echo(toString_int(x))
|   return 0
