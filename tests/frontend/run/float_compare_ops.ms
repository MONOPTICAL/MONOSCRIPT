// expect-output: lt
// expect-output: le
// expect-output: ne
// expect-output: gt
// expect-output: ge
// expect-output: eq
[i32]main()
|   float a = 1.5
|   float b = 2.5
|   if a < b
|   |   echo("lt")
|   if a <= b
|   |   echo("le")
|   if a != b
|   |   echo("ne")
|   if b > a
|   |   echo("gt")
|   if b >= a
|   |   echo("ge")
|   if a == a
|   |   echo("eq")
|   if a > b
|   |   echo("BAD")
|   return 0
