// expect-output: 0
[i32]f(array<i32>: a)
|   return len(a)

[i32]main()
|   echo(toString_int(f([])))
|   return 0
