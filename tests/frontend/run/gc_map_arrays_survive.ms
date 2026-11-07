// expect-output: 2
[void]churn()
|   i32 i = 0
|   while i < 200000
|   |   array<i32> junk = [-1,-1]
|   |   i = i + 1

[i32]main()
|   map<i32,array<i32>> m = {1: [1,2]}
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   echo(toString_int(m[1][1]))
|   return 0
