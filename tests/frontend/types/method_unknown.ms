// expect-error: not found in struct
[struct]P
|   i32 a = 1

[i32]main()
|   P p = P()
|   return p.nope()
