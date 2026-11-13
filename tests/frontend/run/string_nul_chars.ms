// expect-runtime-error: strings cannot contain a NUL byte
[i32]main()
|   array<char> chars = ['a', 0->char, 'b']
|   echo(cstr(chars))
|   return 0
