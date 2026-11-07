// expect-error: 6: used before it is assigned
[i32]main()
|   final i32 x
|   if 1 == 1
|   |   x = 1
|   return x
