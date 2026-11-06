// expect-output: 3
[i32]sum(array<i32>: a)
|   i32 s = 0
|   for x in a
|   |   s = s + x
|   return s

[i32]main()
|   echo(toString_int(sum([1,2])))
|   return 0
