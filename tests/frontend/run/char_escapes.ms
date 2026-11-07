// expect-output: 10
// expect-output: 9
// expect-output: 13
// expect-output: 0
// expect-output: 39
// expect-output: 34
// expect-output: 92
// Проверка через код символа
[i32]main()
|   echo(cstr('\n'->i32))
|   echo(cstr('\t'->i32))
|   echo(cstr('\r'->i32))
|   echo(cstr('\0'->i32))
|   echo(cstr('\''->i32))
|   echo(cstr('"'->i32))
|   echo(cstr('\\'->i32))
|   return 0
