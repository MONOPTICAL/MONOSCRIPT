// expect-output: 1.5
// expect-output: 3.0
// expect-output: 0.1
// expect-output: -0.25
// expect-output: 1.7e+09
// expect-output: inf
// expect-output: nan
// Та же кратчайшая запись, что у f64: большая потеря точности теперь видна
[i32]main()
|   float a = 1.5
|   echo(cstr(a))
|   float b = 3.0
|   echo(cstr(b))
|   float c = 0.1
|   echo(cstr(c))
|   float d = -0.25
|   echo(cstr(d))
|   float e = 1700000000.5
|   echo(cstr(e))
|   float z = 0.0
|   echo(cstr(1.0 / z))
|   echo(cstr(z / z))
|   return 0
