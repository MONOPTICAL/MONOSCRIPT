// expect-output: 2
[i32]main()
|   i32 k = 8
|   i32 n = 0
|   while k > 1
|   |   k = k / 2
|   |   if k == 2
|   |   |   n = k
|   echo(toString_int(n))
|   return 0
