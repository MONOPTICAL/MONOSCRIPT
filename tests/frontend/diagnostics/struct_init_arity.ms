// expect-error: Constructor P expects 1 arguments, got 2
[struct]P
|   i32 x = 0
|   [void]init(i32: x)
|   |   self.x = x

[i32]main()
|   P p = P(1, 2)
|   return 0
