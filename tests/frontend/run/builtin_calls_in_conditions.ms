// expect-output: len
// expect-output: while
// expect-output: has
// expect-output: 3
// expect-output: 5
// Встроенных функций нет в таблице: раньше условие падало с Function not found
[i32]main()
|   array<i32> a = [1,2]
|   map<string,i32> m = {"k": 1}
|   if len(a) == 2
|   |   echo("len")
|   while len(a) > 1
|   |   echo("while")
|   |   break
|   if has(m, "k") and len(m) == 1
|   |   echo("has")
|   i32 x = len(a) + 1
|   echo(toString_int(x))
|   i64 w = len(a) + 3
|   echo(toString_long(w))
|   return 0
