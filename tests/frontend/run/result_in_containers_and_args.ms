// expect-output: 2
// expect-output: ok
// expect-output: 1
[i1]good(result<i32>: r)
|   return ?r

[result<i32>]one()
|   return ok(1)

[i32]main()
|   array<result<i32>> rs = [one(), one()]
|   echo(cstr(len(rs)))
|   if good(one())
|   |   echo("ok")
|   map<string, result<i32>> m = {"a": one()}
|   result<i32> got = m["a"]
|   echo(cstr(got.value))
|   return 0
