// expect-output: a
// expect-output: b
// expect-output: new
// Строка вычисляется один раз
[i32]main()
|   string s = "ab"
|   for c in s
|   |   echo(cstr(c))
|   |   s = "new"
|   echo(s)
|   return 0
