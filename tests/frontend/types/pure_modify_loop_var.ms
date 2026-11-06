// expect-error: cannot modify object
[struct]P
|   i32 age = 1

[void]f(array<P>: ps) @pure
|   for p in ps
|   |   p.age = 5

[i32]main()
|   return 0
