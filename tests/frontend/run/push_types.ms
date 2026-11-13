// expect-output: b
// expect-output: 2.5
// expect-output: 5000000000
// expect-output: 1
// expect-output: z
// expect-output: 7
[struct]P
|   i32 v = 7

[i32]main()
|   array<string> s = ["a"]
|   push(s, "b")
|   echo(s[1])
|   array<f64> d = []
|   push(d, 2.5)
|   echo(cstr(d[0]))
|   array<i64> w = []
|   push(w, 5000000000)
|   echo(cstr(w[0]))
|   array<array<i32>> n = []
|   push(n, [1])
|   echo(cstr(n[0][0]))
|   array<char> c = []
|   push(c, 'z')
|   echo(cstr(c[0]))
|   array<P> ps = []
|   push(ps, P())
|   echo(cstr(ps[0].v))
|   return 0
