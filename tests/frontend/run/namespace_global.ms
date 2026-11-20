// expect-output: 6
// expect-output: 7
// expect-output: 1
[namespace] Xyi
|   i32 x = 5
|   [i32]bump()
|   |   x = x + 1
|   |   return x

i32 x = 1

[i32]main()
|   echo(toString_int(Xyi.bump()))
|   echo(toString_int(Xyi.x + x))
|   echo(toString_int(x))
|   return 0
