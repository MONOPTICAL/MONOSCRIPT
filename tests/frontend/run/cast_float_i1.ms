// expect-output: =true
// expect-output: =true
// expect-output: =false
// expect-output: =false
// expect-output: =false
// Истина это всё, что после отбрасывания дробной части не ноль
[i32]main()
|   float a = 1.0
|   float b = 2.5
|   float c = 0.0
|   float d = 0.5
|   float e = -0.5
|   echo("=" + a->i1)
|   echo("=" + b->i1)
|   echo("=" + c->i1)
|   echo("=" + d->i1)
|   echo("=" + e->i1)
|   return 0
