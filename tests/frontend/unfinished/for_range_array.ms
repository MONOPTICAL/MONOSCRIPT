// expect-error: range expects an integer
[i32]main()
|   array<i32> arr = [1,2]
|   for i in range(arr)
|   |   echo("x")
|   return 0
