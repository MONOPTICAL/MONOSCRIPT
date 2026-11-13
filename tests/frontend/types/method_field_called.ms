// expect-error: is a field of struct
[struct]P
|   i32 a = 1

[i32]main()
|   P p = P()
|   return p.a()
