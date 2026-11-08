// expect-output: 0
// expect-output: 2
// expect-output: -1
// expect-output: 1
// expect-output: -1
use
|> mono.core.arrays

[i32]main()
|   array<i32> a = [7, 8, 9]
|   echo(cstr(indexOf(a, 7)? else -2))
|   echo(cstr(indexOf(a, 9)? else -2))
|   echo(cstr(indexOf(a, 1)? else -2))
|   array<string> dup = ["a", "b", "b"]
|   echo(cstr(indexOf(dup, "b")? else -2))
|   array<i32> empty = []
|   echo(cstr(indexOf(empty, 1)? else -2))
|   return 0
