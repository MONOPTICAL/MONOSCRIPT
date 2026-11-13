// expect-error: 4: Array index must be an integer, got string
[i32]main()
|   array<i32> a = [1]
|   return a["x"]
