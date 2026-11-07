// expect-output: substr: range is outside the string
// expect-output: true
[i32]main()
|   attempt
|   |   echo(substr("abc", -1, 1))
|   panic first
|   |   for i in range(3)
|   |   |   attempt
|   |   |   |   array<i32> a = []
|   |   |   |   n ^= a[i]
|   |   |   panic next
|   |   |   |   gc_collect()
|   |   gc_collect()
|   |   echo(first.message)
|   |   echo("" + (first.line > 0))
|   return 0
