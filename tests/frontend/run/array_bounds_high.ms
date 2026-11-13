// expect-runtime-error: index 3 out of bounds (len 3) at line 5
[i32]main()
|   array<i32> a = [1,2,3]
|   i32 i = 3
|   echo(toString_int(a[i]))
|   return 0
