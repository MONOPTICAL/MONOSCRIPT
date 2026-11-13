// expect-error: 3: for expects an array, map, string or range
[i32]main()
|   for x in [1,2]
|   |   echo("BAD")
|   return 0
