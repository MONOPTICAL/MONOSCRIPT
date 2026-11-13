// expect-output: stack overflow
// expect-output: true
// expect-output: stack overflow
[i32]deep(i32: n)
|   return deep(n + 1) + 1
[i32]main()
|   attempt
|   |   echo("" + deep(0))
|   panic info
|   |   echo(info.message)
|   |   echo("" + (info.line > 0))
|   attempt
|   |   echo("" + deep(0))
|   panic again
|   |   echo(again.message)
|   return 0
