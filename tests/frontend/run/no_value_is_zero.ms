// expect-output: =0
// expect-output: =0.000000
// expect-output: =!
// expect-output: =false
// expect-output: =0
// expect-output: =5
// Раньше сюда записывался указатель: мусор, а строка падала
i32 g

[i32]main()
|   i32 x
|   float f
|   string s
|   i1 b
|   echo("=" + x)
|   echo("=" + f)
|   echo("=" + s + "!")
|   echo("=" + b)
|   echo("=" + g)
|   x = 5
|   echo("=" + x)
|   return 0
