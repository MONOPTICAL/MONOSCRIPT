// expect: ok
[i32]a() @pure
|   return b()

[i32]b() @pure
|   return 1

[i32]main()
|   return 0
