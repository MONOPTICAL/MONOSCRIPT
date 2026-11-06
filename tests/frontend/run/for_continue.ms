// expect-output: 4
[i32]main()
|   i32 s = 0
|   for i in range(4)
|   |   if i == 2
|   |   |   continue
|   |   s = s + i
|   echo(toString_int(s))
|   return 0
