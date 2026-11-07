// expect-output: n=4
[string]show(i32: n) @pure @strict
|   return "n=" + cstr(n)

[i32]main()
|   echo(show(4))
|   return 0
