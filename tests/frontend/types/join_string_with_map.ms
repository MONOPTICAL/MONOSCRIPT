// expect-error: 4: Cannot join string with map
[i32]main()
|   map<string,i32> m = {}
|   echo("x" + m)
|   return 0
