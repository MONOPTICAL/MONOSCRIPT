// expect: ok
// xfail: return в ветке теряется вместе с копией контекста, Statements.cpp:462
[i32]f(i32: a)
|   if a > 1
|   |   return 1
|   else
|   |   return 2

[i32]main()
|   return 0
