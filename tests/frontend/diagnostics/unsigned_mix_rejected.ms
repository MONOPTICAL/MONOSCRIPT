// expect-error: Cannot mix u32 and i32
[i32]main()
|   u32 a = 5
|   i32 n = 5
|   u32 y = a + n
|   return 0
