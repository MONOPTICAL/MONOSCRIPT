// expect-output: hello
[i32]main()
|   array<char> x = []
|   push(x, 'h')
|   push(x, 'e')
|   push(x, 'l')
|   push(x, 'l')
|   push(x, 'o')
|   echo(cstr(x))
|   return 0
