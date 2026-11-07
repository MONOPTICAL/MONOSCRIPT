// expect-output: 7
// expect-output: 8
// expect-output: 9
// expect-output: caught
// expect-output: done
// expect-output: after loop
[i32]work(i32: mode)
|   attempt
|   |   if mode == 0: return 7
|   |   if mode == 1: return toInt("invalid")?
|   |   return 1 / 0
|   else error
|   |   return 8
|   panic info
|   |   return 9
[i32]main()
|   for i in range(3)
|   |   echo("" + work(i))
|   for i in range(4)
|   |   attempt
|   |   |   if i == 0: continue
|   |   |   if i == 2: break
|   |   |   n ^= 1 / 0
|   |   panic info
|   |   |   echo("caught")
|   |   echo("done")
|   attempt
|   |   n ^= 1 / 0
|   panic info
|   |   echo("after loop")
|   return 0
