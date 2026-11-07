// expect-error: Map key must be an integer or string, got float
[map<float,i32>]f()
|   return {}

[i32]main()
|   return 0
