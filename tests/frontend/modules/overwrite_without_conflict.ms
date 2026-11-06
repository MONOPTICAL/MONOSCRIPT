// expect: ok
[i32]mine() @overwrite
|   return 1

[i32]main()
|   return mine()
