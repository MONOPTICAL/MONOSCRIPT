// expect-error: copy expects 1 argument
[i32]main()
|   array<i32> a = [1]
|   array<i32> b = copy(a, a)
|   return 0
