// expect-output: true
// expect-output: false
// expect-output: =a
// expect-output: =ab
[i32]main()
|   i1 t = 1
|   i1 f = 0
|   echo(toString_bool(t))
|   echo(toString_bool(f))
|   echo("=" + toString_char('a'))
|   echo("=" + toString_char('a') + toString_char('b'))
|   return 0
