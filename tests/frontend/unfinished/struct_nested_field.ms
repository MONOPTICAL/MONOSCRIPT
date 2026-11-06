// expect: ok
[struct]In
|   i32 v = 1

[struct]Out
|   In inner = In()

[i32]main()
|   Out o = Out()
|   return o.inner.v
