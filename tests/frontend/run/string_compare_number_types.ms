// expect-output: ok1
// expect-output: ok2
// expect-output: ok3
// Число сравнивается со строкой как текст: i64 раньше обрезался до i32
[i32]main()
|   i64 big = 5000000000
|   if "5000000000" == big
|   |   echo("ok1")
|   array<i32> a = [7]
|   if a[0] == "7"
|   |   echo("ok2")
|   float f = 1.5
|   if "1.500000" == f
|   |   echo("ok3")
|   return 0
