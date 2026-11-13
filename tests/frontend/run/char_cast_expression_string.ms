// expect-output: HELLO
// expect-output: =A
// expect-output: A=
// expect-output: A
// expect-output: A
[string]upper(string: s)
|   string out = ""
|   for c in s
|   |   if c >= 'a' and c <= 'z'
|   |   |   out = out + (c->i32 - 32)->char
|   |   else
|   |   |   out = out + c
|   return out

[i32]main()
|   echo(upper("hello"))
|   i32 n = 64
|   echo("=" + (n + 1)->char)
|   echo((n + 1)->char + "=")
|   echo(cstr((n + 1)->char))
|   i32 negative = -65
|   echo("" + (-negative)->char)
|   return 0
