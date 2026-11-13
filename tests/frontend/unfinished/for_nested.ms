// expect: ok
[i32]main()
|   array<array<i32>> g = [[1],[2]]
|   for row in g
|   |   for c in row
|   |   |   i32 y = c
|   return 0
