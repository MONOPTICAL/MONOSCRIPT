// expect-error: Pure function cannot call
[i32]pure() @pure
|   return impure()

[i32]impure()
|   return 1

[i32]main()
|   return 0
