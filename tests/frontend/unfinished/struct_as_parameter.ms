// expect: ok
[struct]P
|   i32 age = 1

[i32]g(P: p)
|   return p.age

[i32]main()
|   return 0
