// expect: ok
[i32]main()
|   string s = "a"
|   string t = "b"
|   if s != t and s == "a"
|   |   echo("y")
|   else if !(s == t) or t < s
|   |   echo("n")
|   while s == t
|   |   break
|   return 0
