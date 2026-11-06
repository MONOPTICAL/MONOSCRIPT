// expect-output: 4
[array<array<i32>>]mk()
|   return [[1,2],[3,4]]

[i32]main()
|   array<array<i32>> a = mk()
|   echo(toString_int(a[1][1]))
|   return 0
