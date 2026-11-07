// expect-error: expected func<i32, i32>, got func<string, i32>
[i32]bad(string: s)
|   return 1

[i32]apply(func: f(i32: x) -> i32, i32: v)
|   return f(v)

[i32]main()
|   return apply(bad, 1)
