// expect-output: false
// expect-output: division by zero
// expect-output: 2
use
|> mono.core.arrays

// Паника в колбэке становится err, следующие элементы не обрабатываются
i32 calls = 0

[i32]boom(i32: x)
|   calls = calls + 1
|   return 10 / x

[i32]main()
|   array<i32> a = [1, 0, 5]
|   r ^= mapped(a, boom)
|   echo(cstr(r.ok))
|   echo(r.error)
|   echo(cstr(calls))
|   return 0
