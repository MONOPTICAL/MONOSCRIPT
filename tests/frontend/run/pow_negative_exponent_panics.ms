// expect-runtime-error: negative exponent in integer power
[i32]main()
|   i32 b = 2
|   i32 e = 0 - 1
|   return b ** e
