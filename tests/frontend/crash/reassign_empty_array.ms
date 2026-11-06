// expect: ok
// xfail: присваивание пустого литерала разыменовывает нулевой BlockNode, Statements.cpp:372
[i32]main()
|   array<i32> a = [1]
|   a = []
|   return 0
