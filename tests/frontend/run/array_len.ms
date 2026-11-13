// expect-output: 3
// expect-output: 0
[i32]main()
|   array<i32> a = [1,2,3]
|   echo(toString_int(len(a)))
|   array<i32> e = []
|   echo(toString_int(len(e)))
|   return 0
