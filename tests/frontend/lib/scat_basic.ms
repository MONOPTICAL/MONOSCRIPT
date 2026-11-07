// expect-output: ab
// expect-output: =
// expect-output: =x
// expect-output: =x
// expect-output: abcd
[i32]main()
|   echo(scat("a", "b"))
|   echo("=" + scat("", ""))
|   echo("=" + scat("x", ""))
|   echo("=" + scat("", "x"))
|   echo(scat(scat("a", "b"), scat("c", "d")))
|   return 0
