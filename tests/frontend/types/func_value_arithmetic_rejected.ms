// expect-error:
[i32]id(i32: x)
|   return x

[i32]main()
|   i32 n = id + 1
|   return n
