// expect-output: 8080
// expect-output: 7
i32 port = toInt("invalid")? else 8080
[result<i32>]n()
|   return ok(7)
i32 number = n()? else 0
[i32]main()
|   echo("" + port)
|   echo("" + number)
|   return 0
