// expect-error: Cannot reassign constant variable: c
[i32]main()
|   const i32 c = 5
|   c += 1
|   return 0
