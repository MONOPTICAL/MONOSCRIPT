// expect-output: 2
// expect-output: 4
// expect-output: =1
// expect-output: =2
// expect-output: 0
// expect-output: 1
// expect-output: x
use
|> mono.core.arrays

[i32]twice(i32: x)
|   return x * 2

[string]label(i32: x)
|   return "=" + x

[i32]main()
|   array<i32> a = [1, 2]
|   for x in mapped(a, twice)
|   |   echo(cstr(x))
|   array<string> fallback = []
|   for s in mapped(a, label)
|   |   echo(s)
|   array<i32> empty = []
|   echo(cstr(len(mapped(empty, twice))))
|   echo(cstr(a[0]))
|   [char]first(string: s)
|   |   return s[0]
|   array<string> words = ["xy"]
|   array<char> noChars = []
|   for c in mapped(words, first)
|   |   echo(cstr(c))
|   return 0
