// expect-error: needs its type arguments from the target
[struct]Box<T>
|   T v

[i32]main()
|   b ^= Box()
|   return 0
