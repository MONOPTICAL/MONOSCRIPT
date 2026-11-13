// expect-output: 5000000000
// expect-output: -3
[i32]main()
|   map<i64,i32> a = {5000000000: 1}
|   for k in a
|   |   echo(toString_long(k))
|   map<i8,i32> b = {-3: 1}
|   for k in b
|   |   i32 x = k
|   |   echo(toString_int(x))
|   return 0
