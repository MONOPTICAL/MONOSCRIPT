// expect-output: done
[i32]main()
|   array<string> a = []
|   for i in range(1000)
|   |   push(a, "x" + i)
|   for i in range(1000)
|   |   pop(a)
|   gc_collect()
|   echo("done")
|   return 0
