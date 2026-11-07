// expect-error: 4: Strings are immutable
[i32]main()
|   string s = "abc"
|   s[0] = 'x'
|   return 0
