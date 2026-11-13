// expect-output: end
[i32]main()
|   array<i32> a = []
|   for x in a
|   |   echo("BAD")
|   echo("end")
|   return 0
