// expect-error: enum values only support == and !=
[enum]Color
|   Red
|   Blue

[i32]main()
|   i1 b = Color.Red < Color.Blue
|   return 0
