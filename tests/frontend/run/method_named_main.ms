// expect-output: m
// expect-output: 0
[struct]A
|   [string]main()
|   |   return "m"

[i32]main()
|   A a = A()
|   echo(a.main())
|   echo("0")
|   return 0
