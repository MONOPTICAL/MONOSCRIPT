// expect-runtime-error: index -1 out of bounds (len 1) at line 4
[i32]main()
|   array<i32> a = [1]
|   echo(toString_int(a[-1]))
|   return 0
