// expect-output: 3
[i32]f(array<i32>: a)
|   return a[0]

[i32]main()
|   echo(toString_int(f([3,4])))
|   return 0
