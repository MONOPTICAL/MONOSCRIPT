// expect-output: 9
// expect-output: 2
array<i32> g = [1,2]

[i32]main()
|   g[0] = 9
|   gc_collect()
|   echo(toString_int(g[0]))
|   echo(toString_int(len(g)))
|   return 0
