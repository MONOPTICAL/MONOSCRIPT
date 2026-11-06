// expect-output: 42
[i32]copy(i32: x)
|   return 42

[i32]main()
|   echo(toString_int(copy(1)))
|   return 0
