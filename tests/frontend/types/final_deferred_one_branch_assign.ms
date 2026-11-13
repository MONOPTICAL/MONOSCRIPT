// expect-error: 6: already assigned
[i32]main()
|   final i32 x
|   if 1 == 1
|   |   x = 1
|   x = 2
|   return 0
