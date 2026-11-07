// expect-error: 4: Unsupported operand types for comparison
[i32]main()
|   array<i32> a = [1]
|   if "x" == a
|   |   return 1
|   return 0
