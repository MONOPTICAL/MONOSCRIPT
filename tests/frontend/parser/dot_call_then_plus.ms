// expect-parse: (var x (+ (access a (call foo)) 1))
[i32]main()
|   i32 x = a.foo() + 1
|   return 0
