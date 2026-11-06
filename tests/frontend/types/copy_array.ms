// expect: ok
[i32]main()
|   array<i32> a = [1]
|   array<i32> b = copy(a)
|   c ^= copy(a)
|   return 0
