// expect: ok
[struct]P
|   i32 age = 1
|   [i32]get()
|   |   return 1

[i32]main()
|   P p = P()
|   return p.get()
