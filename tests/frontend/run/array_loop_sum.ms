// expect-output: 60
[i32]main()
|   array<i32> a = [10,20,30]
|   i32 s = 0
|   for i in range(len(a))
|   |   s = s + a[i]
|   echo(toString_int(s))
|   return 0
