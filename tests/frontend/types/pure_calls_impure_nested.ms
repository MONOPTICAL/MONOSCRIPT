// expect-error: Pure function cannot call
[void]a() @pure
|   if 1
|   |   echo("nested")

[i32]main()
|   return 0
