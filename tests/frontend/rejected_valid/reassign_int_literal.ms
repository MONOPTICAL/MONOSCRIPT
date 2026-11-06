// expect: ok
// xfail: для литерала берётся его тип, а не тип после каста, Statements.cpp:421
[i32]main()
|   i32 k = 0
|   k = 5
|   return k
