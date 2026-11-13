// expect-error: Fallback: Type mismatch
[i32]main()
|   n ^= toInt("3")? else "bad"
|   return n
