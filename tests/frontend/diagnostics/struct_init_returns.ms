// expect-error: must return void
[struct]P
|   i32 x = 0
|   [i32]init(i32: x)
|   |   return x

[i32]main()
|   P p = P(1)
|   return 0
