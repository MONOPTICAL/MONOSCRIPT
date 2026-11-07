// expect-error: can only call @pure methods
[struct]P
|   i32 a = 1
|   [void]set()
|   |   self.a = 2

[i32]main()
|   final P p = P()
|   p.set()
|   return 0
