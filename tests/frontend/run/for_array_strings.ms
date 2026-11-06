// expect-output: a
// expect-output: b
[i32]main()
|   array<string> a = ["a","b"]
|   for s in a
|   |   echo(s)
|   return 0
