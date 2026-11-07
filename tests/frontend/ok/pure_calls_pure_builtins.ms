// expect: ok
[string]fmt(i32: a) @pure
|   return "n=" + toString_int(len("x") + a)

[i32]main()
|   return 0
