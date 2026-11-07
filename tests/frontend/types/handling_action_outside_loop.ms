// expect-error: Continue statement outside of loop
[i32]main()
|   n ^= toInt("3")? else: continue
|   return 0
