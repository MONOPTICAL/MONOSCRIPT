// expect-error: 4: used before it is assigned
[i32]main()
|   final i32 x
|   x = x + 1
|   return 0
