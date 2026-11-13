// expect-output: 12
[namespace] Xyi
|   [struct]x
|   |   i32 i = 3
|   |   [i32]test()
|   |   |   return 7

[i32]get(Xyi.x: p)
|   return p.i

[i32]main()
|   Xyi.x p = Xyi.x()
|   p.i = 5
|   array<Xyi.x> a = [p]
|   echo(toString_int(get(a[0]) + p.test()))
|   return 0
