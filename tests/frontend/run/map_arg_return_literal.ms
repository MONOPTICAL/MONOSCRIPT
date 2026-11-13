// expect-output: 2
// expect-output: 3
// expect-output: 0
[i32]size(map<string,i32>: m)
|   return len(m)

[map<string,i32>]mk()
|   return {"a": 3}

[map<string,i32>]empty()
|   return {}

[i32]main()
|   echo(toString_int(size({"a": 1, "b": 2})))
|   map<string,i32> m = mk()
|   echo(toString_int(m["a"]))
|   echo(toString_int(size(empty())))
|   return 0
