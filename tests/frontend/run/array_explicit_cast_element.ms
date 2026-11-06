// expect-output: 7
[i32]main()
|   i32 x = 7
|   array<i8> a = [x->i8]
|   a[0] = x->i8
|   i32 r = a[0]
|   echo(toString_int(r))
|   return 0
