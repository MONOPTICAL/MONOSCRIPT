// expect-error: 5: Cannot reassign self
[struct]P
|   i32 a = 1
|   [void]bad()
|   |   self = P()

[i32]main()
|   return 0
