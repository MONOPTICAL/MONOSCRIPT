// expect-error: Map key must be an integer, char or string, got float
[struct]P
|   map<float,i32> m = {}

[i32]main()
|   return 0
