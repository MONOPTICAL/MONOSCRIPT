// expect-error: Method P.get expects 0 arguments, got 1
[struct]P
|   [i32]get()
|   |   return 1

[i32]main()
|   P p = P()
|   return p.get(1)
