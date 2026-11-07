// expect-output: 4
// У каждого объекта свой map
[struct]S
|   map<string,i32> m = {"s": 4}

[i32]main()
|   S a = S()
|   S b = S()
|   a.m["s"] = 0
|   echo(toString_int(b.m["s"]))
|   return 0
