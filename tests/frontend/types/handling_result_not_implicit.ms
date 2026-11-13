// expect-error: The result is not used
[i32]main()
|   attempt
|   |   toInt("invalid")
|   else error
|   |   return 1
|   return 0
