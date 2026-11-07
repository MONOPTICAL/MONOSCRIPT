// expect-runtime-error: result has no value: not a digit: z at line 13
// Забыли проверить: паника с исходной причиной
[result<i32>]parseDigit(string: s)
|   if len(s) != 1
|   |   return err(1, "expected one digit, got " + s)
|   char c = s[0]
|   if c < '0' or c > '9'
|   |   return err(2, "not a digit: " + s)
|   return ok(c->i32 - '0'->i32)

[i32]main()
|   result<i32> r = parseDigit("z")
|   return r.value
