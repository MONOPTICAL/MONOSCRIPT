// expect-output: 11
[result<void>]operation()
|   return err(11, "failed")
[void]work()
|   operation()? else: echo("" + error.code); return
|   echo("unreachable")
[i32]main()
|   work()
|   return 0
