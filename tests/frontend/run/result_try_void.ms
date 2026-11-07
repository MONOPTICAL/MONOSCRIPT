// expect-output: done
// expect-output: 9
// expect-output: stop
[result<void>]action(i1: fail)
|   if fail: return err(9, "stop")
|   return ok()
[result<string>]work(i1: fail)
|   action(fail)?
|   result<void> r = action(false)
|   r?
|   return ok("done")
[i32]main()
|   result<string> good = work(false)
|   echo(good.value)
|   result<string> bad = work(true)
|   echo("" + bad.code)
|   echo(bad.error)
|   return 0
