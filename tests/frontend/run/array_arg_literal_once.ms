// expect-output: called
// expect-output: 1
i32 count = 0

[i32]g()
|   echo("called")
|   count = count + 1
|   return count

[i32]f(array<i32>: a)
|   return a[0]

[i32]main()
|   f([g()])
|   echo(toString_int(count))
|   return 0
