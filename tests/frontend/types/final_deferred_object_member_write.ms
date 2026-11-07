// expect-error: 5: Cannot modify member of constant
[i32]main()
|   final array<i32> a
|   a = [1]
|   a[0] = 2
|   return 0
