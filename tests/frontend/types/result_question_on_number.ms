// expect-error: only for result<T>
[i32]main()
|   i32 x = 1
|   if ?x
|   |   return 1
|   return 0
