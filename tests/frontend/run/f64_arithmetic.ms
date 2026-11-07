// expect-output: 1.5
// expect-output: 0.5
// expect-output: 0.5
// expect-output: 2.0
// expect-output: 1.0
// expect-output: yes
[i32]main()
|   f64 a = 1.0
|   f64 b = 0.5
|   echo(toString_f64(a + b))
|   echo(toString_f64(a - b))
|   echo(toString_f64(a * b))
|   echo(toString_f64(a / b))
|   f64 c = 5.0
|   f64 two = 2.0
|   echo(toString_f64(c % two))
|   if a > b and b <= 0.5
|   |   echo("yes")
|   return 0
