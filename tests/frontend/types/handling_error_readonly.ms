// expect-error: Cannot modify member of constant variable
[i32]main()
|   attempt
|   |   n ^= toInt("invalid")?
|   else error
|   |   error.code = 3
|   return 0
