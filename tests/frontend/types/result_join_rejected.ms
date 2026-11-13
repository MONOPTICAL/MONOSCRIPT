// expect-error: Cannot join string with result
[result<i32>]f()
|   return ok(1)

[i32]main()
|   result<i32> r = f()
|   echo("x" + r)
|   return 0
