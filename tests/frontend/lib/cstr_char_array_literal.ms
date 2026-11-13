// expect-output: abc
// expect-output: 3
[i32]main()
|   string d = cstr(['a', 'b', 'c'])
|   echo(d)
|   echo(cstr(len(d)))
|   return 0
