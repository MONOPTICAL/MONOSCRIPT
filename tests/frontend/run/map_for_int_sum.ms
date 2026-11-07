// expect-output: 6
// expect-output: 60
[i32]main()
|   map<i32,i32> m = {1: 10, 2: 20, 3: 30}
|   i32 keys = 0
|   i32 values = 0
|   for k in m
|   |   keys = keys + k
|   |   values = values + m[k]
|   echo(toString_int(keys))
|   echo(toString_int(values))
|   return 0
