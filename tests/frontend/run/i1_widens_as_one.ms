// expect-output: =1.000000
// expect-output: =2
// expect-output: yes
// expect-output: =1
// Истина это 1: со знаком она превращалась в -1
[float]pf(float: x)
|   return x

[i32]main()
|   array<i1> a = [1]
|   echo("=" + pf(a[0]))
|   i64 big = 1
|   i64 r = a[0] + big
|   echo("=" + r)
|   i64 zero = 0
|   if a[0] > zero
|   |   echo("yes")
|   echo("=" + (a[0])->i32)
|   return 0
