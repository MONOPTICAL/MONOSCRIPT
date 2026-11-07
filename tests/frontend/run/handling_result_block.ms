// expect-output: 7
// expect-output: done
// expect-output: 15
// expect-output: failed step
// expect-output: 9
// expect-output: done
[result<void>]step(i1: fail)
|   if fail: return err(15, "failed step")
|   return ok()
[i32]main()
|   for i in range(3)
|   |   attempt
|   |   |   n ^= toInt("7")?
|   |   |   step(i == 1)?
|   |   |   echo("" + (n + i))
|   |   else error
|   |   |   echo("" + error.code)
|   |   |   echo(error.error)
|   |   |   continue
|   |   echo("done")
|   return 0
