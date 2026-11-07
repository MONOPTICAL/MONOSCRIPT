// expect-output: 1700000000.5
// expect-output: 1.7e+09
// Ради этого f64 и нужен: float не держит столько цифр
[i32]main()
|   f64 t = 1700000000.5
|   echo(toString_f64(t))
|   float bad = 1700000000.5
|   echo(toString_float(bad))
|   return 0
