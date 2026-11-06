// expect-output: 5
// expect-output: 7
// Запись младших байт оставляла старшие: 5000000000 превращалось в 4294967301
[i32]main()
|   i64 big = 5000000000
|   i32 y = 5
|   big = y
|   echo(toString_long(big))
|   i8 small = 7
|   big = small
|   echo(toString_long(big))
|   return 0
