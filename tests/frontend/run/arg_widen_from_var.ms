// expect-output: =7.000000
// expect-output: =300
// Переменная в параметре float: раньше в регистр уходило целое как есть
[float]pf(float: x)
|   return x

[i64]pw(i64: x)
|   return x

[i32]main()
|   i32 v = 7
|   echo("=" + pf(v))
|   i16 s = 300
|   echo("=" + pw(s))
|   return 0
