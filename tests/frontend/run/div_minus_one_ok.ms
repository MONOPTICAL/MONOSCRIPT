// expect-output: =-7
// expect-output: =127
// expect-output: =-127
// На -1 делить можно всё, кроме самого маленького числа
[i32]main()
|   i32 a = 7
|   i32 m = -1
|   echo("=" + a / m)
|   i8 small = -127
|   i8 n = -1
|   echo("=" + small / n)
|   i8 big = 127
|   echo("=" + big / n)
|   return 0
