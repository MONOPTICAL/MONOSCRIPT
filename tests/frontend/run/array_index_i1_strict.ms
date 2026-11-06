// expect-output: 20
[i32]main() @strict
|   array<i32> a = [10,20]
|   i32 i = 5
|   echo(toString_int(a[i > 0]))
|   return 0
