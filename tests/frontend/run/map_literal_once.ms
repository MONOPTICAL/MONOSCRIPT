// expect-output: k
// expect-output: v
// expect-output: 2
// Ключ и значение вычисляются один раз, ключ первым
i32 n = 0

[string]k()
|   echo("k")
|   n = n + 1
|   return "a"

[i32]v()
|   echo("v")
|   n = n + 1
|   return 1

[i32]main()
|   map<string,i32> m = {k(): v()}
|   echo(toString_int(n))
|   return 0
