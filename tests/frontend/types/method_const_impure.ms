// expect-error: 9: Constant 'p' can only call @pure methods
[struct]P
|   i32 a = 1
|   [void]set()
|   |   self.a = 2

[i32]main()
|   const P p = P()
|   p.set()
|   return 0
