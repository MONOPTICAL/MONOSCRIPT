// expect-output: 3
// expect-output: 3
// Структура из импортированного модуля
use
|> runlib_struct

[i32]main()
|   LibP a = libMk()
|   echo(toString_int(a.age))
|   LibP b = LibP()
|   echo(toString_int(b.age))
|   return 0
