// expect-runtime-error: split: empty delimiter
[i32]main()
|   array<string> a = split("abc", "")
|   return 0
