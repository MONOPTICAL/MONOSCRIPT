// expect-error: already has a member named
[struct]P
|   i32 a = 1
|   [i32]a()
|   |   return 2

[i32]main()
|   return 0
