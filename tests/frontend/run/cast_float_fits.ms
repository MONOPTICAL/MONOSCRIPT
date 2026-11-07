// expect-output: =127
// expect-output: =-128
// expect-output: =1
// expect-output: =-1
// expect-output: =70000
// Дробная часть отбрасывается, граница считается по целой части
[i32]main()
|   float a = 127.5
|   float b = -128.5
|   float c = 1.9
|   float d = -1.9
|   float e = 70000.0
|   i8 r1 = a->i8
|   i8 r2 = b->i8
|   echo("=" + r1)
|   echo("=" + r2)
|   echo("=" + c->i32)
|   echo("=" + d->i32)
|   echo("=" + e->i32)
|   return 0
