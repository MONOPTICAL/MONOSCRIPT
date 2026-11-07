// expect-output: -17
// expect-output: 42
// expect-output: 12
// expect-output: 0
// expect-output: 0
// expect-output: 7
[i32]main()
|   echo(cstr(toInt("-17")))
|   echo(cstr(toInt("  42")))
|   echo(cstr(toInt("12abc")))
|   echo(cstr(toInt("abc")))
|   echo(cstr(toInt("")))
|   echo(cstr(toInt("+7")))
|   return 0
