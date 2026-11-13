// expect-output: test!
// expect-output: 5
[i32]main()
|   array<char> x = toArray("test")
|   push(x, '!')
|   string s = cstr(x)
|   echo(s)
|   echo(cstr(len(s)))
|   return 0
