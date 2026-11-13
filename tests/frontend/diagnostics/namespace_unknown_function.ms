// expect-error: Function not found: math.g
[namespace] math
|   [i32]f(i32: x)
|   |   return x + 1

[i32]main()
|   return math.g(1)
