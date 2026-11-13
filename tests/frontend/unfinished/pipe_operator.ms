// expect-error: not implemented
[i32]sum(i32: a, i32: b)
|   return a + b

[i32]main()
|   i32 a = 1
|   i32 x = a |> sum(2, 3)
|   return 0
