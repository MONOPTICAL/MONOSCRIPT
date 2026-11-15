// expect-output: 1
// expect-output: 2
// expect-output: 3
// expect-output: a
// expect-output: b
// expect-output: c
// expect-output: -7
// expect-output: 0
// expect-output: 4
// expect-output: 0.5
// expect-output: 2.5
// expect-output: A
// expect-output: z
use
|> mono.core.arrays

[i32]main()
|   array<i32> nums = [3, 1, 2]
|   for x in arrays.sorted(nums)
|   |   echo(cstr(x))
|   array<string> words = ["c", "a", "b"]
|   for w in arrays.sorted(words)
|   |   echo(w)
|   array<i64> wide = [4, -7, 0]
|   for x in arrays.sorted(wide)
|   |   echo(cstr(x))
|   array<f64> reals = [2.5, 0.5]
|   for x in arrays.sorted(reals)
|   |   echo(cstr(x))
|   array<char> letters = ['z', 'A']
|   for c in arrays.sorted(letters)
|   |   echo(cstr(c))
|   return 0
