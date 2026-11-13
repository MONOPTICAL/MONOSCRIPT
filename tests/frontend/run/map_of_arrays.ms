// expect-output: 2
// expect-output: 9
// expect-output: 0
[i32]main()
|   map<string,array<i32>> m = {"a": [1,2]}
|   echo(toString_int(m["a"][1]))
|   m["a"][1] = 9
|   echo(toString_int(m["a"][1]))
|   m["e"] = []
|   echo(toString_int(len(m["e"])))
|   return 0
