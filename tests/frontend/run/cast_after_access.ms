// expect-output: =2
// expect-output: =3
// expect-output: =4
// expect-output: =5
// Каст сразу после цепочки доступа
[struct]P
|   float v = 4.5

[i32]main()
|   array<float> a = [2.5]
|   map<string,float> m = {"k": 5.5}
|   P p = P()
|   echo("=" + a[0]->i32)
|   i32 n = (a[0]->i32) + 1
|   echo("=" + n)
|   echo("=" + p.v->i32)
|   echo("=" + m["k"]->i32)
|   return 0
