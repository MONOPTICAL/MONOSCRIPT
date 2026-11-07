// expect-output: 123
// Настоящая задача: разобрать число из строки по символам
[i32]parse(string: s)
|   i32 n = 0
|   for c in s
|   |   n = n * 10 + (c->i32 - '0'->i32)
|   return n

[i32]main()
|   echo(cstr(parse("123")))
|   return 0
