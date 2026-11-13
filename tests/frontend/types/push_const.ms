// expect-error: Cannot modify member of constant
[i32]main()
|   const array<i32> a = [1]
|   push(a, 2)
|   return 0
