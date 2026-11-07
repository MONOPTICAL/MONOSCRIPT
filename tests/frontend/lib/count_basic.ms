// expect-output: =2
// expect-output: =0
// expect-output: =2
// expect-output: =3
// expect-output: =1
// expect-output: =0
// expect-output: =3
[i32]main()
|   echo("=" + count("hello", "l"))
|   echo("=" + count("hello", "z"))
|   echo("=" + count("abcabc", "abc"))
|   echo("=" + count("a,b,,c", ","))
|   echo("=" + count("abc", "abc"))
|   echo("=" + count("ab", "abc"))
|   echo("=" + count("ааа", "а"))
|   return 0
