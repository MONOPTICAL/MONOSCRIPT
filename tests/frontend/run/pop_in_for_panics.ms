// expect-runtime-error: array changed during iteration at line 4
[i32]main()
|   array<i32> a = [1, 2]
|   for x in a
|   |   pop(a)
|   return 0
