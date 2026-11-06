// expect-output: value
// expect-output: index
// expect-output: 1
// Как в Python: сначала значение, потом индекс
[i32]value()
|   echo("value")
|   return 1

[i32]index()
|   echo("index")
|   return 0

[i32]main()
|   array<i32> a = [0]
|   a[index()] = value()
|   echo(toString_int(a[0]))
|   return 0
