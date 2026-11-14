// expect-error: in generic function
use
|> mono.core.arrays

[struct]P
|   i32 x = 1

[i32]main()
|   array<P> a = [P(), P()]
|   b ^= arrays.sort(a)
|   return 0
