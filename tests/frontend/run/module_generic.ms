// expect-output: a
// expect-output: 1
use
|> runlib_generic

[i32]main()
|   echo(libFirst(["a"]))
|   echo(cstr(libFirst([1])))
|   return 0
