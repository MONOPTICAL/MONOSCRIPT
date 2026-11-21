// expect-error: is overloaded and cannot be passed by name
[string]show(i32: x)
|   return ""

[string]show(string: s)
|   return ""

[i32]ap(func: f(i32: x) -> string)
|   return 0

[i32]main()
|   return ap(show)
