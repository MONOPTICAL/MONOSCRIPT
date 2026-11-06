// expect: ok
[i64]fibonacci(i64: n) @strict @pure
|   if (n == 0)
|   |   return 0
|   else if (n == 1)
|   |   return 1
|   return fibonacci(n - 1) + fibonacci(n - 2)

[i32]main()
|   echo(toString_long(fibonacci(47)))
|   return 0
