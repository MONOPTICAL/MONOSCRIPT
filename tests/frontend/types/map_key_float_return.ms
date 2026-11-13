// expect-error: Map key must be an integer, char or string, got float
[map<float,i32>]f()
|   return {}

[i32]main()
|   return 0
