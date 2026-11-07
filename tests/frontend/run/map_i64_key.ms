// expect-output: 1
// expect-output: 2
// Ключи различаются старшими битами
[i32]main()
|   map<i64,i32> m = {5000000000: 1, 705032704: 2}
|   echo(toString_int(m[5000000000]))
|   echo(toString_int(m[705032704]))
|   return 0
