// expect-output: =3
// expect-output: =45
// expect-output: =4000000000
// expect-output: =3
// expect-output: =3.5
// Операция смотрит на тип после каста
[i32]main()
|   float a = 2.5
|   i32 n = (a->i32) + 1
|   echo("=" + n)
|   i32 b = 300
|   i32 m = (b->i8) + 1
|   echo("=" + m)
|   i32 c = 2000000000
|   i64 w = (c->i64) * 2
|   echo("=" + w)
|   i32 l = 1 + (a->i32)
|   echo("=" + l)
|   float f = (b->float) / 100.0 + 0.5
|   echo("=" + f)
|   return 0
