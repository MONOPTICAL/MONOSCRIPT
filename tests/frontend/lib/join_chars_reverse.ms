// expect-output: olleh
[i32]main()
|   array<char> x = toArray("hello")
|   array<char> r = []
|   i32 i = len(x) - 1
|   while i >= 0
|   |   push(r, x[i])
|   |   i = i - 1
|   echo(cstr(r))
|   return 0
