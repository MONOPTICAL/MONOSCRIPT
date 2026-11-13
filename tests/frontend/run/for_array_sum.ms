// expect-output: 60
[i32]main()
|   array<i32> a = [10,20,30]
|   i32 s = 0
|   for x in a
|   |   s = s + x
|   echo(toString_int(s))
|   return 0
