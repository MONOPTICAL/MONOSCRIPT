// expect-output: 123
[struct]N
|   i32 v = 123
|   i64 w = 1

[void]churn()
|   i32 i = 0
|   while i < 200000
|   |   N junk = N()
|   |   junk.v = 0
|   |   i = i + 1

[i32]main()
|   N o = N()
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   echo(toString_int(o.v))
|   return 0
