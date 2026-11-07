// expect-output: 2
// expect-output: 3
// expect-output: 4
[i32]main()
|   for i in range(2, 5)
|   |   echo(cstr(i))
|   return 0
