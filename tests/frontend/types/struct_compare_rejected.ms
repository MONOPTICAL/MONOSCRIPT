// expect-error: Unsupported operand types for comparison
[struct]P
|   i32 a = 1

[i32]main()
|   P p = P()
|   P q = P()
|   if p == q
|   |   return 1
|   return 0
