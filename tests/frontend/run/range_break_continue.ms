// expect-output: 2
// expect-output: 4
[i32]main()
|   for i in range(1, 9)
|   |   if i == 1 or i == 3
|   |   |   continue
|   |   if i == 5
|   |   |   break
|   |   echo(cstr(i))
|   return 0
