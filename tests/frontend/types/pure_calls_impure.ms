// expect-error:
// xfail: @pure может вызвать echo через реестр, Statements.cpp:596
[void]f() @pure
|   echo("side effect")

[i32]main()
|   return 0
