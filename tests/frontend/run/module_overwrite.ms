// expect-output: 100
// expect-output: 2
use
|> runlib

[i32]helper() @overwrite
|   return 100

[i32]main()
|   echo(toString_int(helper()))
|   echo(toString_int(usesHelper()))
|   return 0
