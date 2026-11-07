// expect-output: yes
// expect-output: no
// ?r это r.ok
[result<i32>]parseDigit(string: s)
|   if len(s) != 1
|   |   return err(1, "expected one digit, got " + s)
|   char c = s[0]
|   if c < '0' or c > '9'
|   |   return err(2, "not a digit: " + s)
|   return ok(c->i32 - '0'->i32)

[i32]main()
|   result<i32> a = parseDigit("5")
|   if ?a
|   |   echo("yes")
|   result<i32> b = parseDigit("z")
|   if ?b
|   |   echo("BAD")
|   else
|   |   echo("no")
|   return 0
