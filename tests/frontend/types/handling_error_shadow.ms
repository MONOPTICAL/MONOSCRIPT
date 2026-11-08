// expect-error: Variable already defined: error
[i32]main()
|   string error = "mine"
|   n ^= toInt("bad")? else: return 1
|   return n
