// expect-error: 7: Cannot join string with P
[struct]P
|   i32 a = 1

[i32]main()
|   P p = P()
|   echo(p + "x")
|   return 0
