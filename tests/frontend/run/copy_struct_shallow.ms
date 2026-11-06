// expect-output: 7
[struct]P
|   array<i32> v = [1]

[i32]main()
|   P a = P()
|   P b = copy(a)
|   b.v[0] = 7
|   echo(toString_int(a.v[0]))
|   return 0
