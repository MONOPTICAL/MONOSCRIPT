// expect-output: 5
[array<i32>]mk()
|   array<i32> a = [4,5]
|   return a

[i32]main()
|   array<i32> b = mk()
|   gc_collect()
|   echo(toString_int(b[1]))
|   return 0
