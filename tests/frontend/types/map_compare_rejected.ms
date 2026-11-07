// expect-error: Unsupported operand types for comparison
[i32]main()
|   map<string,i32> a = {}
|   map<string,i32> b = {}
|   if a == b
|   |   return 1
|   return 0
