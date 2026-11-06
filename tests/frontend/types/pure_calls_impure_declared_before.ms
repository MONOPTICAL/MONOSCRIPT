// expect-error: Pure function cannot call
[void]a() @pure
|   b()

[void]b()
|   echo("x")

[i32]main()
|   return 0
