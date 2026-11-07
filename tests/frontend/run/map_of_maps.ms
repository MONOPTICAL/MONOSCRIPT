// expect-output: 1
// expect-output: 2
[i32]main()
|   map<string,map<string,i32>> m = {"a": {"x": 1}}
|   echo(toString_int(m["a"]["x"]))
|   m["a"]["y"] = 2
|   echo(toString_int(m["a"]["y"]))
|   return 0
