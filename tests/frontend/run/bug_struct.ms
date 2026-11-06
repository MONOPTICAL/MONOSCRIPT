// expect-output: 1
// xfail: структуры не реализованы в кодгене, ASTGen.cpp
[struct]P
|   i32 age = 1

[i32]main()
|   P p = P()
|   echo(toString_int(p.age))
|   return 0
