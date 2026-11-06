// expect-output: 2
// xfail: доступ через [] не реализован в кодгене, ASTGen.cpp
[i32]main()
|   array<i32> a = [1,2]
|   echo(toString_int(a[1]))
|   return 0
