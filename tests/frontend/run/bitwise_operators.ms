// expect-output: 8
// expect-output: 15
// expect-output: 9
// expect-output: 48
// expect-output: 3
// expect-output: -13
// expect-output: -4
// expect-output: true
// expect-output: 1
// expect-output: 200
// expect-output: 1099511627776
// expect-output: 6
[i32]main()
|   i32 x = 12
|   echo(cstr(x band 10))
|   echo(cstr(x bor 3))
|   echo(cstr(x xor 5))
|   echo(cstr(x shl 2))
|   echo(cstr(x shr 2))
|   echo(cstr(bnot x))
|   i32 neg = 0 - 16
|   echo(cstr(neg shr 2))
|   echo(cstr(x band 1 == 0))
|   echo(cstr(1 bor 2 band 0))
|   i32 big = 200 band 255
|   echo(cstr(big))
|   i64 w = 1
|   echo(cstr(w shl 40))
|   echo(cstr(1 + 2 shl 1))
|   return 0
