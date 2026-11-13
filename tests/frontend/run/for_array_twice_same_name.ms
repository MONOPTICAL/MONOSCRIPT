// expect-output: 3
[i32]main()
|   array<i32> a = [1]
|   i32 s = 0
|   for x in a
|   |   s = s + x
|   array<i32> b = [2]
|   for x in b
|   |   s = s + x
|   echo(toString_int(s))
|   return 0
