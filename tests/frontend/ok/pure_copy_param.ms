// expect: ok
[struct]P
|   i32 age = 1

[i32]f(P: x) @pure
|   P q = copy(x)
|   q.age = 5
|   return q.age

[i32]main()
|   return 0
