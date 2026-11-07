// expect-error: 4: String index must be an integer, got f64
[i32]main()
|   string s = "abc"
|   echo(cstr(s[1.5]))
|   return 0
