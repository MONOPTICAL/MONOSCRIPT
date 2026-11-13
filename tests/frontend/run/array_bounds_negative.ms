// expect-runtime-error: index -1 out of bounds (len 3) at line 5
[i32]main()
|   array<i32> a = [1,2,3]
|   i32 i = -1
|   echo(toString_int(a[i]))
|   return 0
