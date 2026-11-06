// expect-output: 4
[array<i32>]mk(i32: x)
|   return [x,x+1]

[i32]main()
|   array<array<i32>> a = [mk(1),mk(3)]
|   echo(toString_int(a[1][1]))
|   return 0
