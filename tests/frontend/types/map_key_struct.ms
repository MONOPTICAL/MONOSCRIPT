// expect-error: Map key must be an integer, char or string, got P
[struct]P
|   i32 a = 1

[i32]main()
|   map<P,i32> m = {}
|   return 0
