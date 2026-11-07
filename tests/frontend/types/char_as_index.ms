// expect-error: 5: Array index must be an integer, got char
[i32]main()
|   array<i32> a = [1]
|   char c = 'a'
|   return a[c]
