// expect-runtime-error: substr: range is outside the string
[i32]main()
|   echo(substr("abc", 0, -1))
|   return 0
