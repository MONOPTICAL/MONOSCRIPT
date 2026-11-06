// expect-output: 3
[void]churn()
|   i32 i = 0
|   while i < 200000
|   |   array<i32> junk = [7, 7]
|   |   i = i + 1

[i32]main()
|   array<array<i32>> a = [[1,2],[3,4]]
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   echo(toString_int(a[1][0]))
|   return 0
