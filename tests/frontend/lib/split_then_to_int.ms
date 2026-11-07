// expect-output: 60
[i32]main()
|   i32 sum = 0
|   for p in split("10 20 30", " ")
|   |   sum = sum + toInt(p)
|   echo(cstr(sum))
|   return 0
