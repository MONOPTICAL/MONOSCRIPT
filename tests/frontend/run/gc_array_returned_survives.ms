// expect-output: 5
[array<i32>]mk()
|   return [4,5]

[void]churn()
|   i32 i = 0
|   while i < 200000
|   |   array<i32> junk = [9,9]
|   |   i = i + 1

[i32]main()
|   array<array<i32>> rows = [mk(), mk()]
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   echo(toString_int(rows[1][1]))
|   return 0
