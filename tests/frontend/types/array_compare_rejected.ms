// expect-error: 5: Unsupported operand types for comparison
[i32]main()
|   array<i32> a = [1]
|   array<i32> b = [1]
|   if a == b
|   |   return 1
|   return 0
