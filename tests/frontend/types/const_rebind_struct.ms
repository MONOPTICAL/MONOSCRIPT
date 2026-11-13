// expect-error: constant
[struct]P
|   i32 age = 1

[i32]main()
|   const P p = P()
|   p = P()
|   return 0
