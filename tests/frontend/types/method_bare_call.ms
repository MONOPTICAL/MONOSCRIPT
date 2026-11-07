// expect-error: Function not found: other
[struct]P
|   [i32]other()
|   |   return 1
|   [i32]get()
|   |   return other()

[i32]main()
|   return 0
