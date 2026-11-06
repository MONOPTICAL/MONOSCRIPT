// expect-error: new reference
[struct]In
|   i32 v = 1

[struct]Out
|   In i = In()

[void]f(In: x) @pure
|   Out o = Out()
|   o.i = x

[i32]main()
|   return 0
