// expect-output: ok
// xfail: функция с массивом в return падает в кодгене, ASTGen.cpp
[array<i32>]f()
|   return [1]

[i32]main()
|   echo("ok")
|   return 0
