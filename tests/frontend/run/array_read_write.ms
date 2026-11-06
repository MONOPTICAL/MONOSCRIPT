// expect-output: 1
// expect-output: 9
// expect-output: 3
[i32]main()
|   array<i32> a = [1,2,3]
|   echo(toString_int(a[0]))
|   a[1] = 9
|   echo(toString_int(a[1]))
|   echo(toString_int(a[2]))
|   return 0
