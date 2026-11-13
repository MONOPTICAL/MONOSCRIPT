// expect-output: 25000
// expect-output: 150000
// Паника и восстановление в горячем цикле: состояние после attempt не должно портиться
[i64]risky(i64: k)
|   array<i64> a = [1, 2, 3]
|   if k % 4 == 0
|   |   return a[k % 7 + 3]
|   return a[k % 3]

[i32]main()
|   i64 caught = 0
|   i64 sum = 0
|   for i in range(100000)
|   |   i64 k = i
|   |   attempt
|   |   |   sum = sum + risky(k)
|   |   panic info
|   |   |   caught = caught + 1
|   echo(cstr(caught))
|   echo(cstr(sum))
|   return 0
