// expect-error: 4: used before it is assigned
[i32]main()
|   final array<i32> a
|   echo(toString_int(len(a)))
|   return 0
