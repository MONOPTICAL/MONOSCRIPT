// expect-error: Cannot modify member of constant
[i32]main()
|   const map<string,i32> m = {"a": 1}
|   del(m, "a")
|   return 0
