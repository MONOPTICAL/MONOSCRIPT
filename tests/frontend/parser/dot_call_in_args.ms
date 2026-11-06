// expect-parse: (call f (access a (call foo)) 2)
[i32]main()
|   f(a.foo(), 2)
|   return 0
