// expect-error: must return a value
[i32]work()
|   attempt
|   |   return toInt("invalid")?
|   else error
|   |   echo(error.error)
[i32]main()
|   return 0
