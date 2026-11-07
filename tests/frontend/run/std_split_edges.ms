// expect-output: =3
// expect-output: =[]
// expect-output: =1
// expect-output: =2
// expect-output: =2
// expect-output: =b
// Пустые части сохраняются, разделитель может быть длинным
[i32]main()
|   array<string> a = split("x,,y", ",")
|   echo("=" + len(a))
|   echo("=[" + a[1] + "]")
|   array<string> b = split("abc", ",")
|   echo("=" + len(b))
|   array<string> c = split("a,", ",")
|   echo("=" + len(c))
|   array<string> d = split("a--b", "--")
|   echo("=" + len(d))
|   echo("=" + d[1])
|   return 0
