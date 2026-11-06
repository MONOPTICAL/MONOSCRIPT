// expect-output: 0
[array<i32>]mk()
|   return []

[i32]main()
|   array<i32> a = mk()
|   echo(toString_int(len(a)))
|   return 0
