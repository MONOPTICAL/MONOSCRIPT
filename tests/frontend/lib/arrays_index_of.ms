// expect-output: 0
// expect-output: 2
// expect-output: -1
// expect-output: 1
// expect-output: -1
use
|> mono.core.arrays

[i32]main()
|   array<i32> a = [7, 8, 9]
|   echo(cstr(arrays.indexOf(a, 7)))
|   echo(cstr(arrays.indexOf(a, 9)))
|   echo(cstr(arrays.indexOf(a, 1)))
|   array<string> dup = ["a", "b", "b"]
|   echo(cstr(arrays.indexOf(dup, "b")))
|   array<i32> empty = []
|   echo(cstr(arrays.indexOf(empty, 1)))
|   return 0
