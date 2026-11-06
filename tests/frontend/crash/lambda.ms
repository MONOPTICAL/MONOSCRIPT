// expect: no-crash
// xfail: обход лямбды пустой, Expressions.cpp:130
[i32]main()
|   f ^= [i32](i32: a) -> a + 1
|   return 0
