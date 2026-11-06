// expect-output: r=8
[i32]f()
|   return 7

[i32]main()
|   echo("r=" + (f() + 1))
|   return 0
