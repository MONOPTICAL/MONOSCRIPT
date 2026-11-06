// expect-output: 100
// expect-output: 2
use
|> runlib -> helper : lh

[i32]helper()
|   return 100

[i32]main()
|   echo(toString_int(helper()))
|   echo(toString_int(lh()))
|   return 0
