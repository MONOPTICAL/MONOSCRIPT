// expect-output: 10
// expect-output: 30
[void]churn()
|   i32 i = 0
|   while i < 200000
|   |   array<i32> junk = [i, i, i, i]
|   |   i = i + 1

[i32]main()
|   array<i32> a = [10,20,30]
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   echo(toString_int(a[0]))
|   echo(toString_int(a[2]))
|   return 0
