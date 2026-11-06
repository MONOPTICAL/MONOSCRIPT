// expect-output: 2
[i32]main()
|   i32 last = 0
|   for i in range(10)
|   |   if i == 2
|   |   |   last = i
|   |   |   break
|   echo(toString_int(last))
|   return 0
