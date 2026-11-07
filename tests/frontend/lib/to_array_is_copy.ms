// expect-output: abc
// expect-output: Xbc
// Массив — копия, исходная строка не меняется
[i32]main()
|   string s = "abc"
|   array<char> x = toArray(s)
|   x[0] = 'X'
|   echo(s)
|   echo(joinChars(x))
|   return 0
