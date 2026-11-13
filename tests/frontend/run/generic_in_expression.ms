// expect-output: 3
// expect-output: yes
// expect-output: v=2
[T]id<T>(T: x)
|   return x

[i32]main()
|   i32 a = id(1) + id(2)
|   echo(cstr(a))
|   if id(1) < id(2)
|   |   echo("yes")
|   echo("v=" + id(2))
|   return 0
