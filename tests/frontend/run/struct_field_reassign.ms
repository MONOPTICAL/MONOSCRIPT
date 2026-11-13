// expect-output: 4
[struct]In
|   i32 v = 1

[struct]Out
|   In inner = In()

[i32]main()
|   Out o = Out()
|   In n = In()
|   n.v = 4
|   o.inner = n
|   echo(toString_int(o.inner.v))
|   return 0
