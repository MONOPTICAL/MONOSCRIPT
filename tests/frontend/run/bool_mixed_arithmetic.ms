// expect-output: =2
// expect-output: =6
// expect-output: =1.5
[i32]main()
|   x ^= 1
|   i32 n = 1
|   i32 r = x + n
|   echo("=" + r)
|   i64 w = 5
|   i64 q = w + x
|   echo("=" + q)
|   float f = 0.5
|   float g = x + f
|   echo("=" + g)
|   return 0
