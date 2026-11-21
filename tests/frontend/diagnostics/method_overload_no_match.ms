// expect-error: No overload of P.show accepts
[struct]P
|   [string]show(i32: x)
|   |   return ""
|   [string]show(string: s)
|   |   return ""

[i32]main()
|   P p = P()
|   _ ^= p.show(p)
|   return 0
