// expect-output: 0
[i32]main()
|   array<i32> a = []
|   array<i32> b = copy(a)
|   echo(toString_int(len(b)))
|   return 0
