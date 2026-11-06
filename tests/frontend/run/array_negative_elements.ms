// expect-output: -1
// expect-output: -2
// expect-output: 2
[i32]main()
|   array<i32> a = [-1,-2,1+1]
|   echo(toString_int(a[0]))
|   echo(toString_int(a[1]))
|   echo(toString_int(a[2]))
|   return 0
