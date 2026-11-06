// expect-output: -5
// expect-output: 300
[i32]main()
|   array<i8> a = [1,-5]
|   array<i16> b = [300,2]
|   i32 x = a[1]
|   i32 y = b[0]
|   echo(toString_int(x))
|   echo(toString_int(y))
|   return 0
