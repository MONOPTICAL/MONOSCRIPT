// expect: ok
[i32]main()
|   for i in range(5)
|   |   if i == 1
|   |   |   continue
|   |   break
|   return 0
