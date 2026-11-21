// expect-output: 7
[namespace] Xyi
|   [struct]x
|   |   i32 i = 0
|   |   [i32]test()
|   |   |   return 7
|   [i32]run()
|   |   x p = x()
|   |   return p.test()

[i32]main()
|   echo(toString_int(Xyi.run()))
|   return 0
