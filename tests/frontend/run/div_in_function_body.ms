// expect-output: 4
// expect-output: 0
[i32]half(i32: a)
|   return a / 2

[i32]rest(i32: a)
|   i32 r = a % 2
|   return r

[i32]main()
|   echo(toString_int(half(8)))
|   echo(toString_int(rest(8)))
|   return 0
