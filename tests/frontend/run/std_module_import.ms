// expect-output: hello mono!
// expect-output: 7
use
|> std._test.hello

[i32]main()
|   echo(greet("mono"))
|   echo("" + twice(7))
|   return 0
