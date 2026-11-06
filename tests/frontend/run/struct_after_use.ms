// expect-output: 1
// Структура объявлена ниже места использования
[i32]main()
|   P p = P()
|   echo(toString_int(p.age))
|   return 0

[struct]P
|   i32 age = 1
