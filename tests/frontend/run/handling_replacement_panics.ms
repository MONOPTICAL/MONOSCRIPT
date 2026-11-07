// expect-output: division by zero
[i32]replacement()
|   return 1 / 0
[i32]main()
|   attempt
|   |   n ^= toInt("invalid")? else replacement()
|   else error
|   |   echo("wrong result channel")
|   panic info
|   |   echo(info.message)
|   return 0
