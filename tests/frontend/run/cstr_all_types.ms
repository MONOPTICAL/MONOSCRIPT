// expect-output: =true
// expect-output: =-5
// expect-output: =300
// expect-output: =70000
// expect-output: =5000000000
// expect-output: =1.5
// expect-output: =0.1
// expect-output: =s
[i32]main()
|   i1 b = 1
|   i8 c = -5
|   i16 d = 300
|   i32 e = 70000
|   i64 f = 5000000000
|   float g = 1.5
|   f64 h = 0.1
|   string s = "s"
|   echo("=" + cstr(b))
|   echo("=" + cstr(c))
|   echo("=" + cstr(d))
|   echo("=" + cstr(e))
|   echo("=" + cstr(f))
|   echo("=" + cstr(g))
|   echo("=" + cstr(h))
|   echo("=" + cstr(s))
|   return 0
