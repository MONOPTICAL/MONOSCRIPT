// expect-error: cannot modify object
[void]f(map<string,i32>: m) @pure
|   del(m, "a")

[i32]main()
|   return 0
