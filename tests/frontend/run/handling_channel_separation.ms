// expect-output: division by zero
// expect-output: ordinary
// expect-output: 13
// expect-output: division by zero
[result<i32>]work(i1: runtime)
|   if runtime: return ok(1 / 0)
|   return err(13, "ordinary")
[result<i32>]propagate()
|   attempt
|   |   n ^= work(false)?
|   panic info
|   |   echo("not a result error")
|   return ok(0)
[i32]main()
|   for i in range(2)
|   |   attempt
|   |   |   n ^= work(i == 0)?
|   |   else error
|   |   |   echo(error.error)
|   |   panic info
|   |   |   echo(info.message)
|   result<i32> r = propagate()
|   echo("" + r.code)
|   attempt
|   |   n ^= work(true)? else 8080
|   panic info
|   |   echo(info.message)
|   return 0
