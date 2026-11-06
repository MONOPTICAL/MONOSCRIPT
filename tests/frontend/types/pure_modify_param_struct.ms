// expect-error: cannot modify object
[struct]P
|   i32 age = 1

[void]f(P: x) @pure
|   x.age = 5

[i32]main()
|   return 0
