// expect-output: 1
// expect-output: 2
// expect-output: 3
// expect-output: 4
[i32]main()
|   map<i32,i32> m = {0: 1, 1: 2, -1: 3}
|   echo(toString_int(m[0]))
|   echo(toString_int(m[1]))
|   echo(toString_int(m[-1]))
|   i32 k = 1000000
|   m[k] = 4
|   echo(toString_int(m[1000000]))
|   return 0
