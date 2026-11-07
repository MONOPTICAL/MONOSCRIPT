// expect-output: =a
// expect-output: =b
// expect-output: 2
// expect-output: end
// expect-output: 3
[i32]main()
|   string s = "ab"
|   i32 n = 0
|   for c in s
|   |   echo("=" + c)
|   |   n = n + 1
|   echo(cstr(n))
|   for c in ""
|   |   echo("BAD")
|   echo("end")
|   i32 k = 0
|   for c in "xyz"
|   |   k = k + 1
|   echo(cstr(k))
|   return 0
