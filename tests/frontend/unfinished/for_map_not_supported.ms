// expect: ok
[i32]main()
|   map<string,i32> m = {"a": 1}
|   for i in m
|   |   echo("x")
|   return 0
