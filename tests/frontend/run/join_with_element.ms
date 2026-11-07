// expect-output: a=5
// expect-output: f=1.500000
// expect-output: b=true
// expect-output: w=5000000000
// expect-output: 5=l
// expect-output: m=7
// expect-output: p=9
// Число из элемента, значения map и поля раньше уходило в scat как указатель: падение или мусор
[struct]P
|   i32 v = 9

[i32]main()
|   array<i32> a = [5]
|   array<float> f = [1.5]
|   array<i1> b = [1]
|   array<i64> w = [5000000000]
|   map<string,i32> m = {"k": 7}
|   P p = P()
|   echo("a=" + a[0])
|   echo("f=" + f[0])
|   echo("b=" + b[0])
|   echo("w=" + w[0])
|   echo(a[0] + "=l")
|   echo("m=" + m["k"])
|   echo("p=" + p.v)
|   return 0
