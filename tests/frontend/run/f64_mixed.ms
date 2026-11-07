// expect-output: 2.5
// expect-output: 1.75
// expect-output: 3.5
// expect-output: yes
// float с f64 считается в f64, целое тоже
[i32]main()
|   f64 d = 0.5
|   i32 n = 2
|   echo(toString_f64(n + d))
|   float f = 1.25
|   echo(toString_f64(f + d))
|   i64 w = 3
|   echo(toString_f64(w + d))
|   if f < d + 1
|   |   echo("yes")
|   return 0
