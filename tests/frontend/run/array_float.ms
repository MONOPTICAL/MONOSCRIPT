// expect-output: 1.000000
// expect-output: 2.500000
// expect-output: 7.000000
[i32]main()
|   array<float> a = [1,2.5]
|   echo(toString_float(a[0]))
|   echo(toString_float(a[1]))
|   a[0] = 7
|   echo(toString_float(a[0]))
|   return 0
