// expect-output: division by zero
// expect-output: pop from an empty array
// expect-output: division by zero
// expect-output: division by zero
// expect-output: after
[i32]main()
|   attempt
|   |   attempt
|   |   |   n ^= 1 / 0
|   |   panic first
|   |   |   echo(first.message)
|   |   |   attempt
|   |   |   |   array<i32> a = []
|   |   |   |   n ^= pop(a)
|   |   |   panic second
|   |   |   |   echo(second.message)
|   |   |   echo(first.message)
|   |   |   n ^= 2 / 0
|   panic outer
|   |   echo(outer.message)
|   echo("after")
|   return 0
