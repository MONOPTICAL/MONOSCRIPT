// expect-output: 0.1
// expect-output: 0.2
// expect-output: 0.5
f64 g = 0.1

[f64]twice(f64: x)
|   return x * 2

[f64]half(i32: x)
|   return x

[i32]main()
|   echo(toString_f64(g))
|   echo(toString_f64(twice(g)))
|   f64 p = half(1)
|   echo(toString_f64(p / 2))
|   return 0
