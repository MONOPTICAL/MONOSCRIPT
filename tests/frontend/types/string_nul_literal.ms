// expect-error: Strings cannot contain a NUL byte
[i32]main()
|   string s = "a\0b"
|   return len(s)
