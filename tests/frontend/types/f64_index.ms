// expect-error: Array index must be an integer, got f64
[i32]main()
|   array<i32> a = [1]
|   f64 i = 0.0
|   return a[i]
