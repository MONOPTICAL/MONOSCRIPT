// expect-output: 3
// expect-output: 4
// expect-output: 5
[struct]S
|   map<string,i32> m = {"s": 4}
|   i32 n = 5

[i32]main()
|   array<map<string,i32>> a = [{"k": 3}]
|   echo(toString_int(a[0]["k"]))
|   S s = S()
|   echo(toString_int(s.m["s"]))
|   map<string,S> o = {"v": s}
|   echo(toString_int(o["v"].n))
|   return 0
