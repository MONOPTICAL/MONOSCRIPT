// expect-output: cba
[string]rev(string: s)
|   string out = ""
|   i32 i = len(s) - 1
|   while i >= 0
|   |   out = out + s[i]
|   |   i = i - 1
|   return out

[i32]main()
|   echo(rev("abc"))
|   return 0
