// expect-runtime-error: index 3 out of bounds (len 1) at line 7
[struct]P
|   array<i32> v = [1]

[i32]main()
|   P p = P()
|   return p.v[3]
