// expect: ok
[i32]a(i32: n)
|   return b(n)

[i32]b(i32: n)
|   return a(n)

[i32]main()
|   return 0
