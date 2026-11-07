// expect-output: 5000000000
// expect-output: 1.500000
// expect-output: true
// expect-output: s
// expect-output: -3
[i32]main()
|   map<string,i64> a = {"k": 5000000000}
|   map<string,float> b = {"k": 1.5}
|   map<string,i1> c = {"k": 1}
|   map<string,string> d = {"k": "s"}
|   map<string,i8> e = {"k": -3}
|   echo(toString_long(a["k"]))
|   echo(toString_float(b["k"]))
|   echo(toString_bool(c["k"]))
|   echo(d["k"])
|   i32 x = e["k"]
|   echo(toString_int(x))
|   return 0
