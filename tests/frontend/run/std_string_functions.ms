// expect-output: b,c
// expect-output: 2
// expect-output: -1
// expect-output: 42
[i32]main()
|   string s = "a,b,c"
|   echo(substr(s, 2, 3))
|   echo(cstr(strfind(s, "b")))
|   echo(cstr(strfind(s, "z")))
|   echo(cstr(toInt("42")))
|   return 0
