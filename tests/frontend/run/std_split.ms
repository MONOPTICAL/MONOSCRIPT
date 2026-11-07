// expect-output: 3
// expect-output: a
// expect-output: bb
// expect-output: c
// expect-output: =a=
// expect-output: =bb=
// expect-output: =c=
[i32]main()
|   array<string> parts = split("a,bb,c", ",")
|   echo(cstr(len(parts)))
|   echo(parts[0])
|   echo(parts[1])
|   echo(parts[2])
|   for p in parts
|   |   echo("=" + p + "=")
|   return 0
