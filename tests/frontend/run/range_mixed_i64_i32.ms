// expect-output: 2
// expect-output: 3
[i32]main()
|   i64 e = 4
|   i32 s = 2
|   for i in range(s, e)
|   |   echo(cstr(i))
|   return 0
