// expect-output: 7
[i32]main()
|   map<string,array<i32>> a = {"x": [1]}
|   map<string,array<i32>> b = copy(a)
|   b["x"][0] = 7
|   echo(toString_int(a["x"][0]))
|   return 0
