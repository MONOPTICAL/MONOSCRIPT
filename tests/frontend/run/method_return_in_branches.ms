// expect-output: neg
// expect-output: pos
[struct]A
|   [string]sign(i32: n)
|   |   if n < 0
|   |   |   return "neg"
|   |   return "pos"

[i32]main()
|   A a = A()
|   echo(a.sign(-1))
|   echo(a.sign(1))
|   return 0
