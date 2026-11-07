// expect-output: 3
// expect-output: 30
// expect-output: 30
// expect-output: 2
// expect-output: 0
[i32]main()
|   array<i32> a = []
|   push(a, 10)
|   push(a, 20)
|   push(a, 30)
|   echo(cstr(len(a)))
|   echo(cstr(a[2]))
|   echo(cstr(pop(a)))
|   echo(cstr(len(a)))
|   pop(a)
|   pop(a)
|   echo(cstr(len(a)))
|   return 0
