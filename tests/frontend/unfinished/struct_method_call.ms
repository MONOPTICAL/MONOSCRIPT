// expect: ok
// xfail: поиск члена смотрит только поля, а объявить переменную структуры нельзя, NotFinished.cpp:50 и Parser.cpp:149
[struct]P
|   i32 age = 1
|   [i32]get()
|   |   return 1

[i32]main()
|   P p = P()
|   return p.get()
