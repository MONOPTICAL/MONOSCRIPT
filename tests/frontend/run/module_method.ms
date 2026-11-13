// expect-output: 6
use
|> runlib_method

[i32]main()
|   LibM m = LibM()
|   echo(cstr(m.get()))
|   return 0
