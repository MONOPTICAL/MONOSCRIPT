// expect-output: yes
// expect-output: 0.6
// expect-output: 0.5
// expect-output: 2.5
// Литерал рядом с float становится float: f == 0.1 должно быть истиной
[float]half(float: x)
|   return x / 2.0

[i32]main()
|   float f = 0.1
|   if f == 0.1
|   |   echo("yes")
|   float g = f + 0.5
|   echo(cstr(g))
|   float h = 0.5
|   echo(cstr(h))
|   float k = half(5.0)
|   echo(cstr(k))
|   return 0
