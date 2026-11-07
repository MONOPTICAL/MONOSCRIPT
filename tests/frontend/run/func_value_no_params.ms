// expect-output: 42
[i32]answer()
|   return 42

[i32]ask(func: f() -> i32)
|   return f()

[i32]main()
|   echo(cstr(ask(answer)))
|   return 0
