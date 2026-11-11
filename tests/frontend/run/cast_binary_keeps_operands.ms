// expect-output: 0
// expect-output: 7000
// expect-output: 666
// expect-output: 1
// expect-output: 101000
// expect-output: 101000
[i32]main()
|   i64 k = 300
|   i8 a = (k % 100)->i8
|   echo(cstr(a))
|   i64 big = 70000
|   i16 b = (big / 10)->i16
|   echo(cstr(b))
|   f64 x = 2.0
|   f64 y = 3.0
|   echo(cstr((x / y * 1000.0)->i64))
|   f64 h = 0.5
|   echo(cstr((h + h)->i64))
|   // Литерал слева: его узкий тип (i16) не должен обрезать k
|   i64 w = 100000
|   echo(cstr((1000 + w)->i64))
|   i32 n = (1000 + w)->i32
|   echo(cstr(n))
|   return 0
