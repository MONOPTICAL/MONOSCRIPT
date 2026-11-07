// expect-output: 20000
// expect-output: 19999
[i32]main()
|   map<string,i32> m = {}
|   for i in range(20000)
|   |   m["key" + i] = i
|   echo(toString_int(len(m)))
|   echo(toString_int(m["key" + 19999]))
|   return 0
