// expect-error: Cannot modify member of constant variable
[i32]main()
|   attempt
|   |   n ^= 1 / 0
|   panic info
|   |   info.message = "changed"
|   return 0
