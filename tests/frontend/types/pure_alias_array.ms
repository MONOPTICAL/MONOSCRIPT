// expect-error: new reference
[void]f(array<i32>: x) @pure
|   array<i32> b = x
|   b[0] = 1

[i32]main()
|   return 0
