// expect-output: 1.0
// expect-output: 2.5
// expect-output: 7.0
[i32]main()
|   array<float> a = [1,2.5]
|   echo(toString_float(a[0]))
|   echo(toString_float(a[1]))
|   a[0] = 7
|   echo(toString_float(a[0]))
|   return 0
