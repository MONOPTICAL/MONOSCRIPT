// expect-output: =1
// expect-output: =[]
// expect-output: =2
// expect-output: =[][]
// expect-output: =2
// expect-output: =[][a]
// expect-output: =1
[i32]main()
|   array<string> a = split("", ",")
|   echo("=" + len(a))
|   echo("=[" + a[0] + "]")
|   array<string> b = split(",", ",")
|   echo("=" + len(b))
|   echo("=[" + b[0] + "][" + b[1] + "]")
|   array<string> c = split("aaa", "aa")
|   echo("=" + len(c))
|   echo("=[" + c[0] + "][" + c[1] + "]")
|   array<string> d = split("ab", "abc")
|   echo("=" + len(d))
|   return 0
