// expect-output: end
[i32]main()
|   map<string,i32> m = {}
|   for k in m
|   |   echo("BAD")
|   echo("end")
|   return 0
