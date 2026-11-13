// expect-output: 3
// expect-output: end
[i32]main()
|   i32 a = 7
|   i32 b = 2
|   if a > 0
|   |   i32 c = a / b
|   |   echo(toString_int(c))
|   echo("end")
|   return 0
