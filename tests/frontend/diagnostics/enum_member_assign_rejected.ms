// expect-error: Cannot reassign constant variable: Color.Red
[enum]Color
|   Red
|   Blue

[i32]main()
|   Color.Red = Color.Blue
|   return 0
