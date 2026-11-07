// expect-error: 4: Operator '?' is not implemented yet for i32
[i32]main()
|   i32 x = 1
|   if ?x
|   |   return 1
|   return 0
