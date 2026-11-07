// expect-output: =2
// expect-output: =0.5
// expect-output: =0.5
// expect-output: =5000000000
[i32]main()
|   f64 a = 2.75
|   echo("=" + a->i32)
|   f64 b = 0.5
|   float f = b->float
|   echo("=" + f)
|   echo("=" + f->f64)
|   f64 big = 5000000000.5
|   echo("=" + big->i64)
|   return 0
