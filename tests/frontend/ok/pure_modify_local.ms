// expect: ok
[i32]f() @pure
|   array<i32> a = [1]
|   a[0] = 2
|   return a[0]

[i32]main()
|   return 0
