// expect-output: 3
[i32]f(array<array<i32>>: a)
|   return a[1][1]

[i32]main()
|   echo(toString_int(f([[1],[2,3]])))
|   return 0
