// expect: ok
[struct]P
|   i32 age = 1

[i32]f(P: x) @pure
|   return x.age

[i32]main()
|   return 0
