// expect-runtime-error: index 5 out of bounds (len 2)
use
|> mono.core.arrays

[i32]main()
|   array<i32> a = [1, 2]
|   _ ^= swapInArray(a, 0, 5)
|   return 0
