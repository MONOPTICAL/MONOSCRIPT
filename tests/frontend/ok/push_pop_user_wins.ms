// expect: ok
[i32]push(i32: a, i32: b)
|   return a + b

[i32]pop(i32: a)
|   return a

[i32]main()
|   return push(1, 2) + pop(3)
