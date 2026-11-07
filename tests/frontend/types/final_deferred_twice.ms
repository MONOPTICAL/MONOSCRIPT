// expect-error: 5: Final variable 'x' is already assigned
[i32]main()
|   final i32 x
|   x = 1
|   x = 2
|   return 0
