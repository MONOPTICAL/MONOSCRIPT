// expect-error: must be called with ()
[struct]P
|   [i32]get()
|   |   return 1

[i32]main()
|   P p = P()
|   i32 x = p.get
|   return x
