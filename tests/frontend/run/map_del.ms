// expect-output: false
// expect-output: 1
// expect-output: 2
// expect-output: 1
[i32]main()
|   map<string,i32> m = {"a": 1, "b": 2}
|   del(m, "a")
|   echo(toString_bool(has(m, "a")))
|   echo(toString_int(len(m)))
|   echo(toString_int(m["b"]))
|   m["a"] = 1
|   echo(toString_int(m["a"]))
|   return 0
