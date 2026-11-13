// expect-error: 4: Array index must be an integer, got f64
[i32]main()
|   array<i32> a = [1]
|   return a[1.5]
