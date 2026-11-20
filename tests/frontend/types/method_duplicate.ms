// expect-error: Method P.a() is already defined
[struct]P
|   [i32]a()
|   |   return 1
|   [i32]a()
|   |   return 2

[i32]main()
|   return 0
