// expect: ok
[struct]P
|   i32 age = 1

[void]f(P: x)
|   x.age = 5

[i32]main()
|   return 0
