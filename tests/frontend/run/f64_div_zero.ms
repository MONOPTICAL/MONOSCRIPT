// expect-output: inf
// expect-output: -inf
// expect-output: nan
[i32]main()
|   f64 z = 0.0
|   f64 one = 1.0
|   f64 m = -1.0
|   echo(toString_f64(one / z))
|   echo(toString_f64(m / z))
|   echo(toString_f64(z / z))
|   return 0
