// expect-error: Cannot assign to the result of a function call
[struct]P
|   [i32]get()
|   |   return 1

[i32]main()
|   P p = P()
|   p.get() = 1
|   return 0
