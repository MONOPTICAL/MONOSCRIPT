// expect-runtime-error: index -1 out of bounds (len 3) at line 4
[i32]main()
|   string s = "abc"
|   echo(cstr(s[-1]))
|   return 0
