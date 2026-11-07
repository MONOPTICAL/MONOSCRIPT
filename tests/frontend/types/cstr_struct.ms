// expect-error: cstr expects a number or string, got P
[struct]P
|   i32 a = 1

[i32]main()
|   P p = P()
|   echo(cstr(p))
|   return 0
