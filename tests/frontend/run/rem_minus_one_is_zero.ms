// expect-output: =0
// expect-output: =0
// expect-output: =-2
// Остаток от деления на -1 это 0 и для самого маленького числа, падать тут незачем
[i32]main()
|   i32 a = -2147483648
|   i32 m = -1
|   echo("=" + a % m)
|   i32 s = 7
|   echo("=" + s % m)
|   i32 three = 3
|   i32 n = -5
|   echo("=" + n % three)
|   return 0
