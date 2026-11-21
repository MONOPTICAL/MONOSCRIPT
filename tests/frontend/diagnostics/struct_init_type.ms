// expect-error: Constructor P
[struct]P
|   i32 x = 0
|   [void]init(i32: x)
|   |   self.x = x

[i32]main()
|   P p = P("a")
|   return 0
