// expect-error: Cannot modify member of constant
[i32]main()
|   const array<i32> a = [1]
|   pop(a)
|   return 0
