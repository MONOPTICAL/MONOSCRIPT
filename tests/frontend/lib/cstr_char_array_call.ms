// expect-output: abc
// expect-output: def
[i32]main()
|   echo(cstr(toArray("abc")))
|   array<char> chars = toArray("def")
|   echo(cstr(chars))
|   return 0
