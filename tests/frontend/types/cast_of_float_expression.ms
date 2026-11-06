// expect: ok
// xfail: каст теряется, когда семантика пересоздаёт BinaryOpNode, Expressions.cpp:10
[i32]main()
|   echo(toString_int((1.5+2.5)->i32))
|   return 0
