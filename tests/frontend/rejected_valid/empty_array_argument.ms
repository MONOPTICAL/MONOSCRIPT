// expect: ok
[array<i32>]id(array<i32>: a)
|   return a

[i32]main()
|   array<i32> b = id([])
|   return 0
