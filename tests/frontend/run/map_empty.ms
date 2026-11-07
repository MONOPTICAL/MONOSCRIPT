// expect-output: 0
// expect-output: 1
[i32]main()
|   map<string,i32> m = {}
|   echo(toString_int(len(m)))
|   m["a"] = 1
|   echo(toString_int(len(m)))
|   return 0
