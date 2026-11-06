// expect-output: 1
// expect-output: 0
// У каждого объекта свой массив
[struct]P
|   array<i32> v = [1]

[i32]main()
|   P a = P()
|   P b = P()
|   a.v[0] = 0
|   echo(toString_int(b.v[0]))
|   echo(toString_int(a.v[0]))
|   return 0
