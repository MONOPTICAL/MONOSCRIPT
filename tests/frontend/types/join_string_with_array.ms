// expect-error: 4: Cannot join string with array<i32>
[i32]main()
|   array<i32> a = [1]
|   echo("x" + a)
|   return 0
