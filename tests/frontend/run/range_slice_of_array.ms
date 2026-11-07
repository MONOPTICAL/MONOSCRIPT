// expect-output: 20
// expect-output: 30
[i32]main()
|   array<i32> a = [10,20,30]
|   for i in range(1, len(a))
|   |   echo(cstr(a[i]))
|   return 0
