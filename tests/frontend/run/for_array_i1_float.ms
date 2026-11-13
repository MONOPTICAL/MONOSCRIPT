// expect-output: true
// expect-output: false
// expect-output: 1.5
[i32]main()
|   array<i1> a = [1,0]
|   for b in a
|   |   echo(toString_bool(b))
|   array<float> f = [1.5]
|   for v in f
|   |   echo(toString_float(v))
|   return 0
