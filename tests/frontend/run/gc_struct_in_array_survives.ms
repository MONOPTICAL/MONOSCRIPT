// expect-output: 77
[struct]N
|   i32 v = 0
|   string pad = "x"

[void]churn()
|   i32 i = 0
|   while i < 200000
|   |   N junk = N()
|   |   junk.v = -1
|   |   i = i + 1

[i32]main()
|   array<N> a = [N(), N()]
|   a[1].v = 77
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   echo(toString_int(a[1].v))
|   return 0
