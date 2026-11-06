// expect-output: 3
// xfail: for по массиву не реализован в кодгене, ASTGen.cpp
[i32]main()
|   array<i32> a = [1,2]
|   i32 n = 0
|   for x in a
|   |   n = n + x
|   echo(toString_int(n))
|   return 0
