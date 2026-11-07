// expect-runtime-error: index 3 out of bounds (len 3) at line 5
[i32]main()
|   string s = "abc"
|   i32 i = 3
|   echo(cstr(s[i]))
|   return 0
