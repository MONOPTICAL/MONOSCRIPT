// expect-error: no constructor with arguments
[struct]P
|   i32 age = 1
|   string name = "x"

[i32]main()
|   P p = P(1)
|   return 0
