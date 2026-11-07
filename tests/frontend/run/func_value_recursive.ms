// expect-output: 120
[i32]fold(array<i32>: a, i32: i, i32: acc, func: f(i32: a, i32: b) -> i32)
|   if i == len(a)
|   |   return acc
|   return fold(a, i + 1, f(acc, a[i]), f)

[i32]mul(i32: a, i32: b)
|   return a * b

[i32]main()
|   echo(cstr(fold([1,2,3,4,5], 0, 1, mul)))
|   return 0
