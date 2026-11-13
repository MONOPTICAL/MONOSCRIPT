// expect-output: tes
// expect-output: 3
// Длина берётся из массива, а не до нуля в памяти
[i32]main()
|   array<char> x = toArray("test")
|   pop(x)
|   string s = cstr(x)
|   echo(s)
|   echo(cstr(len(s)))
|   return 0
