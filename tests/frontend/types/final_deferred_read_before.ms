// expect-error: 4: Final variable 'x' is used before it is assigned
[i32]main()
|   final i32 x
|   echo(toString_int(x))
|   return 0
