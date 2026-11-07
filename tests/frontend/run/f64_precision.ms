// expect-output: 0.1
// expect-output: 0.30000000000000004
// expect-output: 1234567.1234567
// expect-output: 3.0
// expect-output: -0.5
// expect-output: 5000000000.0
// Точность f64 и кратчайшая запись
[i32]main()
|   f64 a = 0.1
|   echo(toString_f64(a))
|   f64 b = 0.2
|   echo(toString_f64(a + b))
|   f64 c = 1234567.1234567
|   echo(toString_f64(c))
|   f64 d = 3
|   echo(toString_f64(d))
|   f64 e = -0.5
|   echo(toString_f64(e))
|   f64 f = 5000000000
|   echo(toString_f64(f))
|   return 0
