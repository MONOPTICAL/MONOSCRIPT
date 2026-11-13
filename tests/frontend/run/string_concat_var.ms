// expect-output: n=5
// expect-output: r=3
// expect-output: b=true
// expect-output: n=1
[i32]main()
|   i32 n = 5
|   i32 a = 2
|   echo("n=" + n)
|   echo("r=" + (a + 1))
|   echo("b=" + true)
|   echo("n=" + 1)
|   return 0
