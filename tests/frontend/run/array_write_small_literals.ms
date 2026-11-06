// expect-output: 1
// expect-output: -1
// expect-output: 0
// Литералы 0 и 1 имеют тип i1, а -1 унарный: раньше это отвергалось
[i32]main()
|   array<i32> a = [5,5,5]
|   a[0] = 1
|   a[1] = -1
|   a[2] = 0
|   echo(toString_int(a[0]))
|   echo(toString_int(a[1]))
|   echo(toString_int(a[2]))
|   return 0
