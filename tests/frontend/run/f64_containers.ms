// expect-output: 0.1
// expect-output: 0.2
// expect-output: 0.5
// expect-output: 0.25
// Элемент занимает 8 байт, а не 4
[struct]P
|   f64 v = 0.25
|   i8 pad = 1

[i32]main()
|   array<f64> a = [0.1, 0.2, 0.3]
|   echo(toString_f64(a[0]))
|   echo(toString_f64(a[1]))
|   a[2] = 0.5
|   echo(toString_f64(a[2]))
|   P p = P()
|   echo(toString_f64(p.v))
|   return 0
