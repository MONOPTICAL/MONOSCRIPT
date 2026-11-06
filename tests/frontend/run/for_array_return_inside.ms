// expect-output: 2
[i32]first(array<i32>: a)
|   for x in a
|   |   if x > 1
|   |   |   return x
|   return 0

[i32]main()
|   echo(toString_int(first([1,2,3])))
|   return 0
