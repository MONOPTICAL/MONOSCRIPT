// expect-output: a
// expect-output: c
// expect-output: b
// expect-output: x
[i32]main()
|   string s = "abc"
|   echo(cstr(s[0]))
|   echo(cstr(s[2]))
|   i64 i = 1
|   echo(cstr(s[i]))
|   array<string> a = ["xy"]
|   echo(cstr(a[0][0]))
|   return 0
