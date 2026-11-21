// expect-error: Method P.show(i32) is already defined
[struct]P
|   [string]show(i32: x)
|   |   return ""
|   [string]show(i32: y)
|   |   return ""

[i32]main()
|   return 0
