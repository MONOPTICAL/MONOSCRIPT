// expect-error: Type mismatch: expected i32, got Color
[enum]Color
|   Red
|   Blue

[i32]main()
|   i32 n = Color.Red
|   return 0
