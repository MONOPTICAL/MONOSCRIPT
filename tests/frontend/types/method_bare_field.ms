// expect-error: 6:
// Поле только через self
[struct]P
|   i32 a = 1
|   [i32]get()
|   |   return a

[i32]main()
|   return 0
