// expect: ok
[i32]f(map<string,i32>: m) @pure
|   if has(m, "a")
|   |   return m["a"] + len(m)
|   return 0

[i32]main()
|   return 0
