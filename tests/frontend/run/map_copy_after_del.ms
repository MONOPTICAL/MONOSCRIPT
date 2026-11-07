// expect-output: 1
// expect-output: 2
[i32]main()
|   map<string,i32> a = {"x": 1, "y": 2}
|   del(a, "x")
|   map<string,i32> b = copy(a)
|   echo(toString_int(len(b)))
|   echo(toString_int(b["y"]))
|   return 0
