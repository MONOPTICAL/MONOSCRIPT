// expect-output: 7
// expect-output: 8
[result<result<i32>>]nested(i1: fail)
|   result<i32> inner = ok(7)
|   if fail: inner = err(8, "inner")
|   return ok(inner)
[result<i32>]work(i1: fail)
|   i32 n = nested(fail)??
|   return ok(n)
[i32]main()
|   result<i32> a = work(false)
|   result<i32> b = work(true)
|   echo("" + a.value)
|   echo("" + b.code)
|   return 0
