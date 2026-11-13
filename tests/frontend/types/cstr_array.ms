// expect-error: 4: cstr expects a number, char or string, got array<i32>
[i32]main()
|   array<i32> a = [1]
|   echo(cstr(a))
|   return 0
