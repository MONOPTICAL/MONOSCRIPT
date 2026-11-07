// expect-output: false branch
// expect-output: 1
[i32]main()
|   i1 flag = false
|   if flag: echo("BAD")
|   else: echo("false branch")
|   flag = true
|   i32 count = 0
|   while flag
|   |   count = count + 1
|   |   flag = false
|   echo("" + count)
|   return 0
