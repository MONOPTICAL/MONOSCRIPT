// expect-output: 1
// expect-output: 2
[array<i32>]mk(i32: n)
|   if n > 0
|   |   return [1]
|   return [1,2]

[i32]main()
|   array<i32> a = mk(1)
|   array<i32> b = mk(0)
|   echo(toString_int(len(a)))
|   echo(toString_int(len(b)))
|   return 0
