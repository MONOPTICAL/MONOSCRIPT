// expect-error: new reference
[struct]P
|   i32 age = 1

[void]f(array<P>: x) @pure
|   P q = x[0]
|   q.age = 5

[i32]main()
|   return 0
