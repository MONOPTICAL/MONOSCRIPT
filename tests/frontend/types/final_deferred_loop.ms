// expect-error: 5: cannot be assigned in a loop
[i32]main()
|   final i32 x
|   for i in range(2)
|   |   x = i
|   return 0
