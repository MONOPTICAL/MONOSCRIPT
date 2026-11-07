// expect-output: =1
// expect-output: =0
// В i1 истина это 1, а не -1: 1 / 1 не переполнение
[i32]main()
|   x ^= 1
|   y ^= 1
|   echo("=" + x / y)
|   echo("=" + x % y)
|   return 0
