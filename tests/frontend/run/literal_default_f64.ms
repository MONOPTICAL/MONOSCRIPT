// expect-output: 0.1
// expect-output: 0.30000000000000004
// expect-output: =1700000000.5
// x ^= 0.1 это f64, самый точный вариант
[i32]main()
|   v ^= 0.1
|   echo(cstr(v))
|   w ^= v + 0.2
|   echo(cstr(w))
|   t ^= 1700000000.5
|   echo("=" + t)
|   return 0
