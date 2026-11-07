// expect-error: The result is not used
[struct]S
|   [result<i32>]f()
|   |   return ok(1)

[i32]main()
|   S s = S()
|   s.f()
|   return 0
