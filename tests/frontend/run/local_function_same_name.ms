// expect-output: 1
// expect-output: 2
[i32]a()
|   [i32]h()
|   |   return 1
|   return h()
[i32]b()
|   [i32]h()
|   |   return 2
|   return h()
[i32]main()
|   echo(cstr(a()))
|   echo(cstr(b()))
|   return 0
