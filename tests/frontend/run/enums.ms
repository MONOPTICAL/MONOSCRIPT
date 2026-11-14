// expect-output: 0 1 10 11
// expect-output: yes
// expect-output: green
// expect-output: 10
// expect-output: 11
// expect-output: 6
[enum]Color
|   Red
|   Green
|   Blue = 10
|   Black

[struct]P
|   Color c = Color.Blue

[namespace] m
|   i32 x = 1

[Color]next(Color: c)
|   if c == Color.Blue
|   |   return Color.Black
|   return Color.Red

[i32]main()
|   echo(cstr(Color.Red->i32) + " " + cstr(Color.Green->i32) + " " + cstr(Color.Blue->i32) + " " + cstr(Color.Black->i32))
|   Color c = Color.Blue
|   if c == Color.Blue and c != Color.Red
|   |   echo("yes")
|   i32 n = 1
|   if n->Color == Color.Green
|   |   echo("green")
|   P p = P()
|   echo(cstr(p.c))
|   echo(cstr(next(c)))
|   m.x = 6
|   echo(cstr(m.x))
|   return 0
