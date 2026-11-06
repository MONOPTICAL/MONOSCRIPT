// expect-output: ok
// xfail: функция с массивом в return даёт ошибку JIT, заголовок массива лежит на стеке, CodeGenContext.cpp:120
[array<i32>]f()
|   return [1]

[i32]main()
|   echo("ok")
|   return 0
