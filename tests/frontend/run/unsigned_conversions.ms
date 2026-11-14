// expect-output: 4294967295
// expect-output: 255
// expect-output: 200.0
// expect-output: g=200
// expect-output: G=250
// expect-output: s=200
// expect-output: v=200
// expect-output: x=200
[struct]P
|   u8 v = 0

u8 G = 250

[u32]widen(u8: b)
|   return b

[i32]main()
|   i8 n = -1
|   u32 w = n->u32
|   echo(cstr(w))
|   f64 f = 255.0
|   u8 b = f->u8
|   echo(cstr(b))
|   u8 c = 200
|   f64 g = c
|   echo(cstr(g))
|   echo("g=" + widen(c))
|   i32 gx = G
|   echo("G=" + gx)
|   u16 s = 1
|   s = c
|   echo("s=" + s)
|   P p = P()
|   p.v = 200
|   echo("v=" + p.v)
|   x ^= c + 0
|   echo("x=" + x)
|   return 0
