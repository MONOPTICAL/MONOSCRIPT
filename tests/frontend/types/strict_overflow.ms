// expect-error:
// xfail: в @strict литерал вообще не проверяется, Statements.cpp:270
[i32]main() @strict
|   i8 x = 1000
|   return 0
