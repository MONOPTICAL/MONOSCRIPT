// expect-output: division by zero
// expect-output: true
// expect-output: alive
// expect-output: 5
[i32]divide(i32: n)
|   return 10 / n
[i32]deep(i32: depth)
|   if depth == 0: return divide(0)
|   return deep(depth - 1) + 1
[i32]main()
|   attempt
|   |   echo("" + deep(4))
|   |   echo("unreachable")
|   panic info
|   |   echo(info.message)
|   |   echo("" + (info.line > 0))
|   echo("alive")
|   attempt
|   |   echo("" + divide(2))
|   panic info
|   |   echo("stale panic")
|   return 0
