// expect-error: 4: Cannot reassign constant variable
[i32]main()
|   final x ^= 1
|   x = 2
|   return 0
