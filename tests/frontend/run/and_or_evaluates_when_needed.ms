// expect-output: called
// expect-output: yes
// expect-output: called
// expect-output: no
// expect-output: true
// Когда ответ не ясен, правая часть вычисляется
i32 calls = 0

[i1]t()
|   echo("called")
|   return 1 == 1

[i1]f()
|   echo("called")
|   return 1 == 0

[i32]main()
|   if 1 == 1 and t()
|   |   echo("yes")
|   if 1 == 0 or f()
|   |   echo("BAD")
|   else
|   |   echo("no")
|   x ^= 1
|   y ^= 0
|   echo(cstr(x or y and x))
|   return 0
