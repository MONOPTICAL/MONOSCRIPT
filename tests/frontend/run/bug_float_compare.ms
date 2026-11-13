// expect-output: true
[i1]f(float: a)
|   return a > 1.5

[i32]main()
|   echo(toString_bool(f(2.5)))
|   return 0
