// expect-error: Function show(i32) is already defined
[string]show(i32: x)
|   return ""

[string]show(string: s)
|   return ""

[string]show(i32: y)
|   return ""

[i32]main()
|   return 0
