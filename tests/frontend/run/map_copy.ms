// expect-output: 1
// expect-output: 2
// expect-output: 1
// expect-output: 5
[i32]main()
|   map<string,i32> a = {"x": 1}
|   map<string,i32> b = copy(a)
|   b["x"] = 5
|   b["y"] = 6
|   echo(toString_int(a["x"]))
|   echo(toString_int(len(b)))
|   echo(toString_int(len(a)))
|   echo(toString_int(b["x"]))
|   return 0
