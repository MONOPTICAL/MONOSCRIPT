// expect-output: =-1073741824
// expect-output: =1
[i32]main()
|   i32 a = -2147483648
|   i32 two = 2
|   echo("=" + a / two)
|   echo("=" + a / a)
|   return 0
