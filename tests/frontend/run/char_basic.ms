// expect-output: a
// expect-output: =b=
// expect-output: 97
// expect-output: A
// expect-output: yes
// expect-output: lt
[i32]main()
|   char c = 'a'
|   echo(cstr(c))
|   char d = 'b'
|   echo("=" + d + "=")
|   echo(cstr(c->i32))
|   echo(cstr(65->char))
|   if c == 'a'
|   |   echo("yes")
|   if c < d
|   |   echo("lt")
|   return 0
