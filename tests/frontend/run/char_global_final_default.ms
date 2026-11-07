// expect-output: g
// expect-output: f
// expect-output: =0
char g = 'g'

[i32]main()
|   echo(cstr(g))
|   final char f
|   f = 'f'
|   echo(cstr(f))
|   char z
|   echo("=" + z->i32)
|   return 0
