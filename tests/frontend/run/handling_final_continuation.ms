// expect-output: 7
[i32]main()
|   final i32 n
|   attempt
|   |   n = toInt("7")?
|   else error
|   |   return 1
|   panic info
|   |   return 2
|   echo("" + n)
|   return 0
