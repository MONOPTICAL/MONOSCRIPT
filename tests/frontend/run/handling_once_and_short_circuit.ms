// expect-output: 1
// expect-output: 2
// expect-output: short
[result<i32>]step(array<i32>: calls)
|   calls[0] = calls[0] + 1
|   return err(1, "bad")
[i32]main()
|   array<i32> calls = [0]
|   n ^= step(calls)? else 4
|   echo("" + calls[0])
|   attempt
|   |   n = step(calls)? + step(calls)?
|   else error
|   |   echo("" + calls[0])
|   attempt
|   |   if false and (1 / 0 == 1): return 1
|   |   if true or (1 / 0 == 1): echo("short")
|   panic info
|   |   echo("wrong")
|   return 0
