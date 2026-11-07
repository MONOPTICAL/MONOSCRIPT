// expect-output: v=0.1
// expect-output: 0.1=v
// expect-output: e=0.25
// expect-output: m=0.5
// expect-output: p=0.75
// expect-output: c=0.1
[struct]P
|   f64 v = 0.75

[f64]g()
|   return 0.1

[i32]main()
|   f64 a = 0.1
|   echo("v=" + a)
|   echo(a + "=v")
|   array<f64> b = [0.25]
|   echo("e=" + b[0])
|   map<string,f64> m = {"k": 0.5}
|   echo("m=" + m["k"])
|   P p = P()
|   echo("p=" + p.v)
|   echo("c=" + g())
|   return 0
