// expect-output: =2
// expect-output: =2
// expect-output: =2.0
// expect-output: =2
// Литералы 1 и 0 это тоже i1
[i32]main()
|   a ^= 1 + 1
|   echo("=" + a)
|   i8 b = 1 + 1
|   i32 c = b
|   echo("=" + c)
|   float f = 1 + 1
|   echo("=" + f)
|   i64 w = 1 + 1
|   echo("=" + w)
|   return 0
