// expect-error: expected func<i32, i1>, got func<i32, i32>
[i32]id(i32: x)
|   return x

[i32]count(func: f(i32: x) -> i1)
|   return 0

[i32]main()
|   return count(id)
