// expect-error: Cannot assign to captured variable k
[i32]main()
|   i32 k = 1
|   func<i32, i32> f = [i32](i32: x):
|   |   k = 5
|   |   return x
|   return 0
