// expect-runtime-error: index 4294967296 out of bounds (len 1) at line 6
// Индекс не должен обрезаться до 32 бит: 2^32 превратился бы в 0
[i32]main()
|   array<i32> a = [1]
|   i64 i = 4294967296
|   echo(toString_int(a[i]))
|   return 0
