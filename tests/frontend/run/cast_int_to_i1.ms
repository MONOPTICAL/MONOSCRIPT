// expect-output: =true
// expect-output: =true
// expect-output: =false
// В i1 истина это всё, что не ноль: обрезка превращала 2 в ложь
[i32]main()
|   i32 a = 2
|   i32 b = -1
|   i32 c = 0
|   echo("=" + a->i1)
|   echo("=" + b->i1)
|   echo("=" + c->i1)
|   return 0
