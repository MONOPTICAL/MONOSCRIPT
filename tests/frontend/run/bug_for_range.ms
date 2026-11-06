// expect-output: 3
// xfail: for не реализован в кодгене, ASTGen.cpp
[i32]main()
|   i32 n = 0
|   for i in range(3)
|   |   n = n + 1
|   echo(toString_int(n))
|   return 0
