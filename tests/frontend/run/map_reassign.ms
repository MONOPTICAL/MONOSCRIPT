// expect-output: 7
// expect-output: 0
// expect-output: 1
[i32]main()
|   map<string,i32> m = {"a": 1}
|   map<string,i32> keep = m
|   m = {"b": 7}
|   echo(toString_int(m["b"]))
|   m = {}
|   echo(toString_int(len(m)))
|   echo(toString_int(keep["a"]))
|   return 0
