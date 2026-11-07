// expect-error: Postfix '?' requires result<T>
[result<i32>]f()
|   i32 n = 7?
|   return ok(n)
[i32]main()
|   return 0
