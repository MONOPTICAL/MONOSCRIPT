// expect-error: 5: cannot be assigned in a loop
[i32]main()
|   final i32 x
|   while 1 == 1
|   |   x = 1
|   return 0
