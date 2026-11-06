// expect-error: 5: use explicit cast
[i32]main()
|   array<i8> a = [1]
|   i32 x = 1
|   a[0] = x
|   return 0
