// expect-output: ok
// xfail: массив через ^= падает в кодгене, причина не найдена, ASTGen.cpp
[i32]main()
|   y ^= [1, 2]
|   echo("ok")
|   return 0
