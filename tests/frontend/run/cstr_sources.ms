// expect-output: 5
// expect-output: 7
// expect-output: 9
// expect-output: 11
// expect-output: x
[struct]P
|   i32 v = 9

[i32]g()
|   return 11

[i32]main()
|   array<i32> a = [5]
|   map<string,i32> m = {"k": 7}
|   P p = P()
|   echo(cstr(a[0]))
|   echo(cstr(m["k"]))
|   echo(cstr(p.v))
|   echo(cstr(g()))
|   array<string> s = ["x"]
|   echo(cstr(s[0]))
|   return 0
