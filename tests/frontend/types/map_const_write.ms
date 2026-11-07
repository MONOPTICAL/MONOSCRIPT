// expect-error: Cannot modify member of constant
[i32]main()
|   const map<string,i32> m = {}
|   m["a"] = 1
|   return 0
