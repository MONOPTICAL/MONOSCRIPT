// expect-output: =[]
// expect-output: =0
[i32]main()
|   array<char> x = []
|   string s = cstr(x)
|   echo("=[" + s + "]")
|   echo("=" + len(s))
|   return 0
