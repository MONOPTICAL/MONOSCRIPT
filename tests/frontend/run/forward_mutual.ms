// expect-output: true
// expect-output: false
[i1]even(i32: n)
|   if n == 0
|   |   return 1 == 1
|   return odd(n - 1)

[i1]odd(i32: n)
|   if n == 0
|   |   return 1 == 0
|   return even(n - 1)

[i32]main()
|   echo(toString_bool(even(10)))
|   echo(toString_bool(even(7)))
|   return 0
