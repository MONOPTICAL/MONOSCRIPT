// expect-error: constant
[i32]main()
|   const array<i32> a = [1]
|   a[0] = 2
|   return 0
