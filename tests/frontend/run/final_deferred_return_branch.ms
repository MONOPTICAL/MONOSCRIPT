// expect-output: 1
// Ветка с return дальше не идёт, поэтому присваивание в другой считается
[i32]f(i32: n)
|   final i32 x
|   if n > 0
|   |   x = 1
|   else
|   |   return 0
|   return x

[i32]g(i32: n)
|   final i32 y
|   if n < 0
|   |   return 0
|   y = 1
|   return y

[i32]main()
|   echo(toString_int(f(1) + g(1) - 1))
|   return 0
