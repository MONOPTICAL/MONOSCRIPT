// expect-output: 5
// xfail: присваивание через [] не реализовано в кодгене, ASTGen.cpp
[i32]main()
|   array<i32> a = [1,2]
|   a[0] = 5
|   echo(toString_int(a[0]))
|   return 0
