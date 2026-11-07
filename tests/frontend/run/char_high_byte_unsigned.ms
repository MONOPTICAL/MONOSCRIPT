// expect-output: 209
// expect-output: 143
// expect-output: gt
// Байт без знака: 0xD1 это 209, а не -47, и он больше ASCII
[i32]main()
|   string s = "я"
|   echo(cstr(s[0]->i32))
|   char c = s[1]
|   echo(cstr(c->i32))
|   if s[0] > 'a'
|   |   echo("gt")
|   return 0
