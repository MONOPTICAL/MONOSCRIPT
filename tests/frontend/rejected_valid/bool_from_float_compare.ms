// expect: ok
// xfail: тип переменной берётся из выражения, а не из объявления, Utils.cpp:61
[i32]main()
|   float a = 1.5
|   float b = 2.5
|   i1 c = a < b
|   echo(toString_bool(c))
|   return 0
