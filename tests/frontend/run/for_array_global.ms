// expect-output: 1
// expect-output: 2
array<i32> g = [1,2]

[i32]main()
|   for x in g
|   |   echo(toString_int(x))
|   return 0
