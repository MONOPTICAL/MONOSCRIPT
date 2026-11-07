// expect-output: 7
// expect-output: 2
[i32]main()
|   i32 x = 7
|   array<i64> a = []
|   push(a, x)
|   echo(cstr(a[0]))
|   array<i32> b = [1]
|   push(b, 1 + 1)
|   echo(cstr(b[1]))
|   return 0
