// expect-error:
[i8]t(i8: a)
|   return a

[i32]main() @strict
|   i64 x = 5000000000
|   i8 y = t(x)
|   return 0
