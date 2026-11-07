// expect-output: =0
// expect-output: =-1
// expect-output: =5000000000
// expect-output: =-9223372036854775808
// expect-output: =1.5
// expect-output: =0.1
// expect-output: =3.0
// expect-output: =0.1
// expect-output: =2.0
[i32]main()
|   echo("=" + toString_int(0))
|   echo("=" + toString_int(-1))
|   i64 big = 5000000000
|   echo("=" + toString_long(big))
|   i64 low = -9223372036854775807 - 1
|   echo("=" + toString_long(low))
|   float f = 1.5
|   echo("=" + toString_float(f))
|   float g = 0.1
|   echo("=" + toString_float(g))
|   f64 h = 3.0
|   echo("=" + toString_f64(h))
|   f64 k = 0.1
|   echo("=" + toString_f64(k))
|   float two = 2.0
|   echo("=" + toString_float(two))
|   return 0
