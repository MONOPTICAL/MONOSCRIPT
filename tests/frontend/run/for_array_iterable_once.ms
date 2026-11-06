// expect-output: mk
// expect-output: 1
// expect-output: 2
[array<i32>]mk()
|   echo("mk")
|   return [1,2]

[i32]main()
|   for x in mk()
|   |   echo(toString_int(x))
|   return 0
