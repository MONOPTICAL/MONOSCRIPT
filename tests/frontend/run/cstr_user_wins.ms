// expect-output: mine
[string]cstr(i32: x)
|   return "mine"

[i32]main()
|   echo(cstr(1))
|   return 0
