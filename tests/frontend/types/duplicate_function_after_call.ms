// expect-error: already defined
[i32]main()
|   return dup()

[i32]dup()
|   return 1

[i32]dup()
|   return 2
