// expect-error: cannot be created
[N]mk()
|   return N()

[struct]N
|   N next = mk()

[i32]main()
|   return 0
