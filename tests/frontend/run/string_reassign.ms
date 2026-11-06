// expect-output: stop
// expect-output: 3
[i32]main()
|   string s = "go"
|   i32 n = 0
|   while s == "go"
|   |   n = n + 1
|   |   if n > 2
|   |   |   s = "stop"
|   echo(s)
|   echo(toString_int(n))
|   return 0
