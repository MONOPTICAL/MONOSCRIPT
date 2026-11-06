// expect-output: 1
// expect-output: 8
// expect-output: 8
[struct]In
|   i32 v = 1

[struct]Out
|   In inner = In()

[i32]main()
|   Out o = Out()
|   echo(toString_int(o.inner.v))
|   o.inner.v = 8
|   In i = o.inner
|   echo(toString_int(i.v))
|   echo(toString_int(o.inner.v))
|   return 0
