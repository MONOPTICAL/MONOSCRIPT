// expect-runtime-error: division by zero
use
|> mono.core.arrays

// Без result паника в колбэке mapped не перехватывается и завершает программу
[i32]boom(i32: x)
|   return 10 / x

[i32]main()
|   array<i32> a = [1, 0, 5]
|   r ^= arrays.mapped(a, boom)
|   return len(r)
