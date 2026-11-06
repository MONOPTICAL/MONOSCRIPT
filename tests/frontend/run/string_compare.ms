// expect-output: eq
// expect-output: lt
[i32]main()
|   string s = "abc"
|   if s == "abc"
|   |   echo("eq")
|   if s < "abd"
|   |   echo("lt")
|   return 0
