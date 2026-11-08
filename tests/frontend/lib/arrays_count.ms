// expect-output: 2
// expect-output: 0
// expect-output: 0
// expect-output: 3
// expect-output: 1
// expect-output: 1
use
|> mono.core.arrays

[i32]main()
|   array<i32> a = [1, 2, 1]
|   echo(cstr(count(a, 1)? else -1))
|   echo(cstr(count(a, 9)? else -1))
|   array<i32> empty = []
|   echo(cstr(count(empty, 1)? else -1))
|   array<string> s = ["x", "x", "x"]
|   echo(cstr(count(s, "x")? else -1))
|   array<i64> wide = [5000000000, 1]
|   echo(cstr(count(wide, 5000000000)? else -1))
|   array<char> c = ['a', 'b']
|   echo(cstr(count(c, 'b')? else -1))
|   return 0
