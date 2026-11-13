// expect-output: 12
// expect-output: bad: not a digit: x
// Передать ошибку наверх руками
[result<i32>]parseDigit(string: s)
|   if len(s) != 1
|   |   return err(1, "expected one digit, got " + s)
|   char c = s[0]
|   if c < '0' or c > '9'
|   |   return err(2, "not a digit: " + s)
|   return ok(c->i32 - '0'->i32)

[result<i32>]twoDigits(string: a, string: b)
|   result<i32> x = parseDigit(a)
|   if !x.ok
|   |   return err(x.code, x.error)
|   result<i32> y = parseDigit(b)
|   if !y.ok
|   |   return y
|   return ok(x.value * 10 + y.value)

[i32]main()
|   result<i32> r = twoDigits("1", "2")
|   echo(cstr(r.value))
|   result<i32> e = twoDigits("1", "x")
|   echo("bad: " + e.error)
|   return 0
