// expect-output: abc
// expect-output: 3
[i32]main()
|   array<char> x = ['a', 'b', 'c']
|   string s = joinChars(x)
|   echo(s)
|   echo(cstr(len(s)))
|   return 0
