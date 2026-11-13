// expect-runtime-error: index 0 out of bounds (len 0) at line 4
[i32]main()
|   array<i32> a = []
|   echo(toString_int(a[0]))
|   return 0
