// expect-output: =2
// expect-output: =5
// expect-output: =0
[i32]main()
|   x ^= 1
|   r ^= x + 1
|   echo("=" + r)
|   i32 n = 1 + x + 3
|   echo("=" + n)
|   y ^= 0
|   i32 z = 0 * y
|   echo("=" + z)
|   return 0
