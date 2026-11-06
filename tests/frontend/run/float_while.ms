// expect-output: 3
[i32]main()
|   float a = 0.5
|   i32 n = 0
|   while a < 3.0
|   |   a = a + 1.0
|   |   n = n + 1
|   echo(toString_int(n))
|   return 0
