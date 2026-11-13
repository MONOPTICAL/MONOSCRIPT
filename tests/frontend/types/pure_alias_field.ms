// expect-error: new reference
[struct]In
|   i32 v = 1

[struct]Out
|   In i = In()

[void]f(Out: x) @pure
|   In q = x.i

[i32]main()
|   return 0
