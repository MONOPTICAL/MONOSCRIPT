// expect-runtime-error: substr: range is outside the string
[i32]main()
|   echo(substr("abc", 4, 0))
|   return 0
