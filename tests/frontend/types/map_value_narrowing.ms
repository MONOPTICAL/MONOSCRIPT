// expect-error: use explicit cast
[i32]main()
|   i32 x = 1
|   map<string,i8> m = {"a": x}
|   return 0
