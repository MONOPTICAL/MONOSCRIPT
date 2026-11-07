// expect-error: 8:
[struct]P
|   [void]v()
|   |   return

[i32]main()
|   P p = P()
|   i32 x = p.v()
|   return x
