// expect-output: =4
// expect-output: =1
// expect-output: =0
// expect-output: =3
// Как в Python: пустая подстрока находится len(s) + 1 раз, "abc".count("") == 4
[i32]main()
|   echo("=" + count("abc", ""))
|   echo("=" + count("", ""))
|   echo("=" + count("", "a"))
|   echo("=" + count("я", ""))
|   return 0
