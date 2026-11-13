// expect: ok
[i32]main()
|   i32 i = 0
|   while i < 10
|   |   i = i + 1
|   |   if i == 5
|   |   |   break
|   return i
