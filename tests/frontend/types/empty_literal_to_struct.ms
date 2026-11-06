// expect-error: Empty literal
[struct]P
|   i32 age = 1

[i32]main()
|   P p = []
|   return 0
