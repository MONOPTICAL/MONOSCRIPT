// expect-output: 2
// expect-output: 0
// expect-output: 1
[i32]main()
|   array<i32> a = [1]
|   array<i32> b = a
|   a = [8,9]
|   echo(toString_int(len(a)))
|   a = []
|   echo(toString_int(len(a)))
|   echo(toString_int(len(b)))
|   return 0
