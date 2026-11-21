// expect-error: No overload of P.init accepts
[struct]P
|   i32 x = 0
|   [void]init(i32: x)
|   |   self.x = x
|   [void]init(i32: x, i32: y)
|   |   self.x = x + y

[i32]main()
|   P p = P("a")
|   return 0
