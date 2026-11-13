// expect-output: 4
// expect-output: 0
// expect-output: 2
[i32]main()
|   echo(cstr(len(toArray("test"))))
|   echo(cstr(len(toArray(""))))
|   echo(cstr(len(toArray("я"))))
|   return 0
