// expect-error: 3: Unsupported operand types for comparison
[i1]less<T>(T: a, T: b)
|   return a < b

[i32]main()
|   less([1], [2])
|   return 0
