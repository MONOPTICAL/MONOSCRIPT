// expect-output: =1 2 3 5 8 

[array<i32>]sort(array<i32>: x)
|   for i in range(len(x))
|   |    for j in range(i + 1, len(x))
|   |    |   if (x[i] > x[j])
|   |    |   | i32 temp = x[i]
|   |    |   | x[i] = x[j]
|   |    |   | x[j] = temp
|   |    
|   return x

[i32]main()
|   array<i32> arr = [5, 3, 8, 1, 2]
|   arr = sort(arr)
|   string result = ""
|   for i in range(len(arr))
|   |   result = result + cstr(arr[i]) + " "
|   
|   echo("=" + result)
|   return 0