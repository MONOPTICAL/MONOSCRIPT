// expect-output: 9
// expect-output: 2
map<string,i32> g = {"a": 1}

[void]add()
|   g["b"] = 9

[i32]main()
|   add()
|   gc_collect()
|   echo(toString_int(g["b"]))
|   echo(toString_int(len(g)))
|   return 0
