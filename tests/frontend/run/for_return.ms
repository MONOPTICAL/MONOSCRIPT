// expect-output: 3
[i32]f()
|   for i in range(10)
|   |   if i == 3
|   |   |   return i
|   return -1

[i32]main()
|   echo(toString_int(f()))
|   return 0
