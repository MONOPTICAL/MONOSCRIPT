// expect-error: Duplicate else handler
[i32]main()
|   attempt
|   |   n ^= toInt("invalid")?
|   else error
|   |   return 1
|   else again
|   |   return 2
|   return 0
