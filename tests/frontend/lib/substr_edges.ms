// expect-output: =[]
// expect-output: =abc
// expect-output: =[]
// expect-output: =a
// expect-output: =c
[i32]main()
|   echo("=[" + substr("abc", 0, 0) + "]")
|   echo("=" + substr("abc", 0, 3))
|   echo("=[" + substr("abc", 3, 0) + "]")
|   echo("=" + substr("abc", 0, 1))
|   echo("=" + substr("abc", 2, 1))
|   return 0
