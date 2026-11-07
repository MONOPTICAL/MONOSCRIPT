// expect-output: =2
// expect-output: =0
// expect-output: =1
// expect-output: =1
// expect-output: =0
// expect-output: =3
// expect-output: =true
// Арифметика над i1 это арифметика над числами: 1 + 1 это 2, а не 0
[i32]main()
|   x ^= 1
|   y ^= 1
|   r ^= x + y
|   echo("=" + r)
|   echo("=" + (x - y))
|   echo("=" + (x * y))
|   echo("=" + (x / y))
|   echo("=" + (x % y))
|   i32 s = x + y + 1
|   echo("=" + s)
|   i1 same = x == y
|   echo("=" + same)
|   return 0
