// expect-output: true
[struct]T
|   f ^= 1
|   s ^= "x"

[i32]main()
|   T t = T()
|   echo(toString_bool(t.f))
|   return 0
