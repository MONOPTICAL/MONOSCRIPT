// expect-output: 5
// expect-output: x
// ok() и err() получают тип от параметра
[i32]valueOr(result<i32>: r, i32: fallback)
|   if ?r
|   |   return r.value
|   return fallback

[string]why(result<i32>: r)
|   return r.error

[i32]main()
|   echo(cstr(valueOr(ok(5), 0)))
|   echo(why(err("x")))
|   return 0
