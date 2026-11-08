// expect-error: Variable not found: k
[i32]main()
|   i32 k = 3
|   [i32]addk(i32: x)
|   |   return x + k
|   return addk(4)
