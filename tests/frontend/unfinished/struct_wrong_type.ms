// expect-error: expected Q, got P
[struct]P
|   i32 age = 1
|   string name = "x"

[struct]Q
|   i32 x = 1

[i32]main()
|   Q q = P()
|   return 0
