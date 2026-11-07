// expect-output: =t
// expect-output: =e
// expect-output: =s
// expect-output: =t
// expect-output: true
[i32]main()
|   array<char> x = toArray("test")
|   for c in x
|   |   echo("=" + toString_char(c))
|   echo(cstr(x[1] == 'e'))
|   return 0
