// expect-output: 1
[i32]main()
|   map<string,i32> a = {}
|   map<string,i32> b = copy(a)
|   b["x"] = 1
|   echo(toString_int(len(b)))
|   return 0
