// expect-error: new reference
[struct]P
|   i32 age = 1

[void]f(P: x) @pure
|   q ^= x

[i32]main()
|   return 0
