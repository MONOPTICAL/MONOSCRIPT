// expect-output: and
// expect-output: or
[i32]main()
|   i32 n = 5
|   if n > 1 and n < 10
|   |   echo("and")
|   if n > 100 or n < 10
|   |   echo("or")
|   return 0
