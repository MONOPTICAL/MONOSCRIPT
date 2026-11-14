// expect-output: 8
// expect-output: 5
// expect-output: 40
// expect-output: -6
// expect-output: n1
// expect-output: n2
// expect-output: n3
// expect-output: 2 2
use
|> mono.core.arrays

[i32]ap(func: f(i32: x) -> i32, i32: v)
|   return f(v)

[i32]dbl(i32: x)
|   return x * 2

[i32]main()
|   echo(cstr(ap(dbl, 4)))
|   echo(cstr(ap([i32](i32: x) -> x + 1, 4)))
|   i32 k = 10
|   echo(cstr(ap([i32](i32: x) -> x * k, 4)))
|   func<i32, i32> h = [i32](i32: x) -> x - k
|   k = 100
|   echo(cstr(h(4)))
|   array<i32> a = [1, 2, 3]
|   for v in arrays.mapped(a, [string](i32: x) -> "n" + x)
|   |   echo(v)
|   array<i32> log = []
|   func<i32, i32> add = [i32](i32: x):
|   |   push(log, x)
|   |   return len(log)
|   add(5)
|   echo(cstr(add(6)) + " " + cstr(len(log)))
|   return 0
