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
|   echo(cstr(arrays.count(a, 1)))
|   echo(cstr(arrays.count(a, 9)))
|   array<i32> empty = []
|   echo(cstr(arrays.count(empty, 1)))
|   array<string> s = ["x", "x", "x"]
|   echo(cstr(arrays.count(s, "x")))
|   array<i64> wide = [5000000000, 1]
|   echo(cstr(arrays.count(wide, 5000000000)))
|   array<char> c = ['a', 'b']
|   echo(cstr(arrays.count(c, 'b')))
|   return 0
