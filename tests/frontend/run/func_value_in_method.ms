// expect-output: 2
[i1]big(i32: x)
|   return x > 10

[struct]Box
|   array<i32> items = [5, 20, 30]
|   [i32]countIf(func: keep(i32: x) -> i1)
|   |   i32 n = 0
|   |   for x in self.items
|   |   |   if keep(x)
|   |   |   |   n = n + 1
|   |   return n

[i32]main()
|   Box b = Box()
|   echo(cstr(b.countIf(big)))
|   return 0
