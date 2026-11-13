// expect-output: 1
// expect-output: 5
[struct]P
|   i32 age = 1
|   string name = "x"

[i32]f(P: x) @pure
|   P q = copy(x)
|   q.age = 5
|   return q.age

[i32]main()
|   P p = P()
|   i32 r = f(p)
|   echo(toString_int(p.age))
|   echo(toString_int(r))
|   return 0
