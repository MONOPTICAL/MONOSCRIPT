// expect-output: 1
// expect-output: 7
// expect-output: 2.000000
[i32]main()
|   i32 x = 7
|   map<string,i64> a = {"k": 1, "v": x}
|   map<string,float> b = {"k": 2}
|   echo(toString_long(a["k"]))
|   echo(toString_long(a["v"]))
|   echo(toString_float(b["k"]))
|   return 0
