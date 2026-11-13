// expect-output: 0
// expect-output: 7
// expect-output: 1 1 2 2
// expect-output: 1 2 3 4
// expect-output: 1 2 3 4
// expect-output: 1 100 true
use
|> mono.core.arrays

[string]show(array<i32>: a)
|   string out = ""
|   for x in a
|   |   if len(out) > 0: out = out + " "
|   |   out = out + cstr(x)
|   return out

[i32]main()
|   array<i32> empty = []
|   echo(cstr(len(sort(empty))))
|   array<i32> one = [7]
|   echo(show(sort(one)))
|   array<i32> dup = [2, 1, 2, 1]
|   echo(show(sort(dup)))
|   array<i32> done = [1, 2, 3, 4]
|   echo(show(sort(done)))
|   array<i32> back = [4, 3, 2, 1]
|   echo(show(sort(back)))
|   array<i32> many = []
|   for i in range(100)
|   |   push(many, 100 - i)
|   sorted ^= sort(many)
|   i1 ascending = true
|   for i in range(1, 100)
|   |   if sorted[i - 1] > sorted[i]: ascending = false
|   echo(cstr(sorted[0]) + " " + cstr(sorted[99]) + " " + cstr(ascending))
|   return 0
