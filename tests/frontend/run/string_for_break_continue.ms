// expect-output: b
[i32]main()
|   for c in "abcd"
|   |   if c == 'a'
|   |   |   continue
|   |   echo(cstr(c))
|   |   break
|   return 0
