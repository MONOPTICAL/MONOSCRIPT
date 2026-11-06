// expect-error: 4: Array index must be an integer, got float
[i32]main()
|   array<i32> a = [1]
|   return a[1.5]
