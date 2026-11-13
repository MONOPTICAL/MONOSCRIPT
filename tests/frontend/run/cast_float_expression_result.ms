// expect-output: 666
// expect-output: 1
// expect-output: 7
// expect-output: 21
// Явный каст применяется к результату, операнды f64 не обрезаются до вычисления
[i32]main()
|   f64 a = 2.0
|   f64 b = 3.0
|   echo(cstr((a / b * 1000.0)->i64))
|   f64 h = 0.5
|   echo(cstr((h + h)->i64))
|   f64 q = 2.7
|   echo(cstr((q * q)->i32))
|   i64 x = 10
|   i64 y = 3
|   echo(cstr((x / y * 7)->i32))
|   return 0
