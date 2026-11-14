// expect-error: No overload of show accepts
[string]show(i32: x)
|   return ""

[string]show(string: s)
|   return ""

[i32]main()
|   _ ^= show(1.5)
|   return 0
