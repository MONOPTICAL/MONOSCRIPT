// expect-output: 20
// expect-output: 20
// expect-output: 10
// expect-output: 20
// Индекс всегда приводится к целому: i1 это 0 или 1
[i32]main()
|   array<i32> a = [10,20]
|   x ^= 1
|   echo(toString_int(a[x]))
|   i32 i = 5
|   echo(toString_int(a[i > 0]))
|   echo(toString_int(a[i < 0]))
|   echo(toString_int(a[1]))
|   return 0
