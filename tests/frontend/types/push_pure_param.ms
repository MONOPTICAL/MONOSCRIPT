// expect-error: Pure function cannot modify object
[void]f(array<i32>: a) @pure
|   push(a, 1)

[i32]main()
|   return 0
