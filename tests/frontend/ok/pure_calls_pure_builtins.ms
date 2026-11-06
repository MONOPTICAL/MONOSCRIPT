// expect: ok
[string]fmt(i32: a) @pure
|   return "n=" + toString_int(sum(a, 1))

[i32]main()
|   return 0
