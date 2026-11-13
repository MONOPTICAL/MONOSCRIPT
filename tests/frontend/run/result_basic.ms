// expect-output: 7
// expect-output: no: not a digit: x
// expect-output: code 2
// expect-output: no: expected one digit, got 12
// expect-output: code 1
[result<i32>]parseDigit(string: s)
|   if len(s) != 1
|   |   return err(1, "expected one digit, got " + s)
|   char c = s[0]
|   if c < '0' or c > '9'
|   |   return err(2, "not a digit: " + s)
|   return ok(c->i32 - '0'->i32)

[void]show(string: s)
|   result<i32> r = parseDigit(s)
|   if r.ok
|   |   echo(cstr(r.value))
|   else
|   |   echo("no: " + r.error)
|   |   echo("code " + r.code)

[i32]main()
|   show("7")
|   show("x")
|   show("12")
|   return 0
