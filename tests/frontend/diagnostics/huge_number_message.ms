// expect-error: 99999999999999999999
// xfail: вся ошибка сводится к слову stoll, Logic.cpp:310
[i32]main()
|   i64 x = 99999999999999999999
|   return 0
