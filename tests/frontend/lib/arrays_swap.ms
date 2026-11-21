// expect-output: 30
// expect-output: 10
// expect-output: 3
// expect-output: b
// expect-output: a
use
|> mono.core.arrays

[i32]main()
|   array<i32> a = [10, 20, 30]
|   arrays.swapInArray(a, 0, 2)
|   echo(cstr(a[0]))
|   echo(cstr(a[2]))
|   arrays.swapInArray(a, 1, 1)
|   echo(cstr(len(a)))
|   array<string> s = ["a", "b"]
|   arrays.swapInArray(s, 0, 1)
|   echo(s[0])
|   echo(s[1])
|   return 0
