// expect-output: 2
[i1]positive(i32: x) @pure
|   return x > 0

[i32]count(array<i32>: a, func: keep(i32: x) -> i1) @pure
|   i32 n = 0
|   for x in a
|   |   if keep(x)
|   |   |   n = n + 1
|   return n

[i32]main()
|   echo(cstr(count([1,-1,5], positive)))
|   return 0
