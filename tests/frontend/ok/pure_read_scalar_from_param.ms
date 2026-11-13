// expect: ok
[struct]P
|   i32 age = 1

[i32]f(P: x) @pure
|   i32 a = x.age
|   return a

[i32]main()
|   return 0
