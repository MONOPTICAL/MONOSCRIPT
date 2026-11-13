// expect-runtime-error: index 5 out of bounds (len 2) at line 4
[i32]main()
|   array<array<i32>> a = [[1,2]]
|   echo(toString_int(a[0][5]))
|   return 0
