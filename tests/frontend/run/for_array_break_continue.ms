// expect-output: 1
// expect-output: 3
[i32]main()
|   array<i32> a = [1,2,3,4,5]
|   for x in a
|   |   if x == 2
|   |   |   continue
|   |   if x == 4
|   |   |   break
|   |   echo(toString_int(x))
|   return 0
