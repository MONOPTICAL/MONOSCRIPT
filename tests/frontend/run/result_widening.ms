// expect-output: 5
// expect-output: 7
// Значение в ok() приводится к T по обычным правилам
[result<i64>]big()
|   i32 x = 5
|   return ok(x)

[result<array<i64>>]arr()
|   return ok([7])

[i32]main()
|   result<i64> r = big()
|   echo(cstr(r.value))
|   result<array<i64>> a = arr()
|   echo(cstr(a.value[0]))
|   return 0
