// expect-error: The result is not used
[result<i32>]f(result<result<i32>>: r)
|   r?
|   return ok(0)
[i32]main()
|   return 0
