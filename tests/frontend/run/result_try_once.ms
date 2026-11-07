// expect-output: 1
// expect-output: 51
// expect-output: 2
// expect-output: 5
[result<i32>]step(array<i32>: calls, i1: fail)
|   calls[0] = calls[0] + 1
|   if fail: return err(51, "stop")
|   return ok(calls[0])
[result<i32>]work(array<i32>: calls, i1: fail)
|   return ok(step(calls, fail)? + step(calls, false)? * 2)
[i32]main()
|   array<i32> calls = [0]
|   result<i32> failed = work(calls, true)
|   echo("" + calls[0])
|   echo("" + failed.code)
|   calls[0] = 0
|   result<i32> good = work(calls, false)
|   echo("" + calls[0])
|   echo("" + good.value)
|   return 0
