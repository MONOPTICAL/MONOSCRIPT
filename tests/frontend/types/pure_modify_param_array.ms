// expect-error: cannot modify object
[void]f(array<i32>: x) @pure
|   x[0] = 7

[i32]main()
|   return 0
