// expect-error: T is both i32 and string
use
|> mono.core.arrays

[i32]main()
|   array<i32> a = [1]
|   n ^= arrays.indexOf(a, "x")
|   return n
