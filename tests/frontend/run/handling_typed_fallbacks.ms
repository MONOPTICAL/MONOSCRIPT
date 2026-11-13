// expect-output: 7
// expect-output: 0
// expect-output: 6
// expect-output: 2.5
// expect-output: 8
[result<T>]fail<T>(T: x)
|   return err(1, "bad")
[struct]P
|   i32 n = 6
[i32]main()
|   map<string, i32> m = fail({"a": 1})? else {"b": 7}
|   echo("" + m["b"])
|   array<i32> a = fail([1])? else []
|   echo("" + len(a))
|   P p = fail(P())? else P()
|   echo("" + p.n)
|   f64 f = fail(1.5)? else 2.5
|   echo("" + f)
|   result<i32> inner = ok(9)
|   result<i32> r = fail(inner)? else ok(8)
|   echo("" + r.value)
|   return 0
