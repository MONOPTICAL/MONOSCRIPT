// expect-output: 1
// expect-output: 2
// expect-output: 9
// expect-output: 7
[i32]main()
|   map<string,i32> m = {"a": 1, "b": 2}
|   echo(toString_int(m["a"]))
|   echo(toString_int(m["b"]))
|   m["a"] = 9
|   echo(toString_int(m["a"]))
|   m["new"] = 7
|   echo(toString_int(m["new"]))
|   return 0
