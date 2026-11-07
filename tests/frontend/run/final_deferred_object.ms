// expect-output: 2
// expect-output: 1
// expect-output: 3
[struct]P
|   i32 v = 3

[i32]main()
|   final array<i32> a
|   a = [1,2]
|   echo(toString_int(len(a)))
|   final map<string,i32> m
|   m = {"k": 1}
|   echo(toString_int(m["k"]))
|   final P p
|   p = P()
|   echo(toString_int(p.v))
|   return 0
