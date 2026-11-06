// expect-output: Test
// expect-output: ab
// expect-output: new
[i32]main()
|   array<string> a = ["Test", "a" + "b"]
|   echo(a[0])
|   echo(a[1])
|   a[0] = "new"
|   echo(a[0])
|   return 0
