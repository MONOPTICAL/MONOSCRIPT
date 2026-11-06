// expect-output: ok
[struct]E
|   i32 unused = 0

[i32]main()
|   E e = E()
|   echo("ok")
|   return 0
