// expect-output: 3
// expect-output: 3.0
// x->T в копии шаблона
[T]conv<T>(T: dummy, i32: x)
|   return x->T

[i32]main()
|   i64 w = 0
|   echo(cstr(conv(w, 3)))
|   f64 d = 0.0
|   echo(cstr(conv(d, 3)))
|   return 0
