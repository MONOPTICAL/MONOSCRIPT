// expect: ok
[struct]P
|   i32 age = 1

[i32]main()
|   P p = P()
|   P q = copy(p)
|   return q.age
