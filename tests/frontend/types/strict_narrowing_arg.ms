// expect-error:
// xfail: @strict разрешает сужение i64 в i8 у аргумента, Statements.cpp:636
[i8]t(i8: a)
|   return a

[i32]main() @strict
|   i64 x = 5000000000
|   i8 y = t(x)
|   return 0
