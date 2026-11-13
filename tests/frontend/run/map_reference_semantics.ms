// expect-output: 5
[void]set(map<string,i32>: m)
|   m["a"] = 5

[i32]main()
|   map<string,i32> a = {"a": 1}
|   map<string,i32> b = a
|   set(b)
|   echo(toString_int(a["a"]))
|   return 0
