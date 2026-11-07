// expect-output: false
// expect-output: 0
[result<i1>]check(array<i32>: calls)
|   calls[0] = calls[0] + 1
|   return err(5, "must not run")
[result<i1>]work(array<i32>: calls)
|   i1 flag = false and check(calls)?
|   return ok(flag)
[i32]main()
|   array<i32> calls = [0]
|   result<i1> value = work(calls)
|   echo("" + value.value)
|   echo("" + calls[0])
|   return 0
