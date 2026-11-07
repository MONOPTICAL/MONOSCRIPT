// expect-output: b
[i32]main()
|   map<string,i32> m = {"a": 1, "b": 2, "c": 3}
|   for k in m
|   |   if k == "a"
|   |   |   continue
|   |   echo(k)
|   |   break
|   return 0
