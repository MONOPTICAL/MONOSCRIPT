// expect-error: 5: Array index must be an integer, got i1
[i32]main()
|   array<i32> a = [1]
|   i32 x = 1
|   return a[x > 0]
