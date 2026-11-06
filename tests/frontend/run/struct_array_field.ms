// expect-output: 0
// expect-output: 2
// expect-output: 9
// expect-output: 1
[struct]P
|   array<i32> v = []

[i32]main()
|   P p = P()
|   echo(toString_int(len(p.v)))
|   p.v = [8,9]
|   echo(toString_int(len(p.v)))
|   echo(toString_int(p.v[1]))
|   p.v = [1]
|   echo(toString_int(len(p.v)))
|   return 0
