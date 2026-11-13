// expect-output: =10
[T]bigger<T>(T: a, T: b)
|   if (a > b) 
|   |   return a
|   return b

[i32]main()
|   i32 a = 5
|   i32 b = 10
|   echo("=" + cstr(bigger(a, b)))
|   return 0