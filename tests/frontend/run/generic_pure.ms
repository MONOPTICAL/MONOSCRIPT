// expect-output: 2
[T]pick<T>(array<T>: a, i32: i) @pure
|   return a[i]

[i32]f(array<i32>: a) @pure
|   return pick(a, 1)

[i32]main()
|   echo(cstr(f([1,2])))
|   return 0
