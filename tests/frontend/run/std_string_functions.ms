// expect-output: 5
// expect-output: b,c
// expect-output: b
// expect-output: 2
// expect-output: -1
// expect-output: 42
[i32]main()
|   string s = "a,b,c"
|   echo(cstr(str_len(s)))
|   echo(substr(s, 2, 3))
|   echo(getIndex(s, 2))
|   echo(cstr(strfind(s, "b")))
|   echo(cstr(strfind(s, "z")))
|   echo(cstr(toInt("42")))
|   return 0
