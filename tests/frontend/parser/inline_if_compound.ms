// expect-error: Inline if/else requires a single simple statement
[i32]main()
|   if true: while true
|   |   break
|   return 0
