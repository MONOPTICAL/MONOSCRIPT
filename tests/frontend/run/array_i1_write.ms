// expect-output: false
// expect-output: true
[i32]main()
|   array<i1> a = [1,1]
|   a[0] = 0
|   echo(toString_bool(a[0]))
|   a[0] = 1 < 2
|   echo(toString_bool(a[0]))
|   return 0
