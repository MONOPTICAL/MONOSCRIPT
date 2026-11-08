// expect-output: 2
// expect-output: true
// expect-output: division by zero
// expect-output: 20
// expect-output: 20
// expect-output: 7
// expect-output: 7
// expect-output: 32
[result<i32>]divide(i32: a, i32: b)
|   if b == 0: return err(1, "division by zero")
|   return ok(a / b)
[array<i32>]mk()
|   return [10, 20, 30]
[struct]P
|   i32 x = 7
|   [result<i32>]get()
|   |   return ok(self.x)
[P]mkP()
|   return P()
[i32]main()
|   echo(cstr(divide(4, 2).value))
|   echo("" + divide(4, 2).ok)
|   echo(divide(4, 0).error)
|   echo("" + mk()[1])
|   echo("" + mk()[1]->i64)
|   echo("" + mkP().x)
|   echo("" + mkP().get().value)
|   echo("" + (divide(4, 2).value + mk()[2]))
|   return 0
