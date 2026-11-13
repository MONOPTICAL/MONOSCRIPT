// expect-output: keep7
// expect-output: err8
[result<string>]make(i32: n)
|   if n == 7
|   |   return ok("keep" + n)
|   return err("err" + n)

[void]churn()
|   i32 i = 0
|   while i < 200000
|   |   string junk = "kill" + i
|   |   i = i + 1

[i32]main()
|   result<string> a = make(7)
|   result<string> b = make(8)
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   echo(a.value)
|   echo(b.error)
|   return 0
