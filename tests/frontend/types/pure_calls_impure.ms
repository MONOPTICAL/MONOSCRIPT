// expect-error:
[void]f() @pure
|   echo("side effect")

[i32]main()
|   return 0
