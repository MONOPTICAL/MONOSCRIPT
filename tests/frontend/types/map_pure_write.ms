// expect-error: cannot modify object
[void]f(map<string,i32>: m) @pure
|   m["a"] = 1

[i32]main()
|   return 0
