// expect-error: 4: Strings are immutable
[i32]main()
|   array<string> a = ["abc"]
|   a[0][0] = 'x'
|   return 0
