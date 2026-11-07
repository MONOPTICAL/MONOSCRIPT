// expect-error: Could not resolve base of chained access: 'error'
[i32]main()
|   n ^= toInt("3")? else: return error.code
|   return error.code
