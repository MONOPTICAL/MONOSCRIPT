// expect-output: 0.6000000000000001
// expect-output: 0.1
[i32]main()
|   array<f64> a = [0.1, 0.2, 0.3]
|   f64 s = 0.0
|   for x in a
|   |   s = s + x
|   echo(toString_f64(s))
|   array<f64> b = copy(a)
|   echo(toString_f64(b[0]))
|   return 0
