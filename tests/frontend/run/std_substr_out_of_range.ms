// expect-runtime-error: substr: range is outside the string
[i32]main()
|   echo(substr("abc", 2, 5))
|   return 0
