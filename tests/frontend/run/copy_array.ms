// expect-output: 1
// expect-output: 2
// expect-output: 3
[i32]main()
|   array<i32> a = [1,2]
|   array<i32> b = copy(a)
|   b[0] = 3
|   echo(toString_int(a[0]))
|   echo(toString_int(len(b)))
|   echo(toString_int(b[0]))
|   return 0
