// expect-output: 2
// expect-output: 1
[struct]S
|   array<i32> v = []

[i32]main()
|   S s = S()
|   push(s.v, 1)
|   push(s.v, 2)
|   echo(cstr(len(s.v)))
|   map<string, array<i32>> m = {"k": []}
|   push(m["k"], 5)
|   echo(cstr(len(m["k"])))
|   return 0
