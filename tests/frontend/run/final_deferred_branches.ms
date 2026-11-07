// expect-output: 2
// expect-output: b
[i32]main()
|   i32 n = 0
|   final i32 x
|   if n > 0
|   |   x = 1
|   else
|   |   x = 2
|   echo(toString_int(x))
|   final string s
|   if n > 5
|   |   s = "a"
|   else if n > 3
|   |   s = "c"
|   else
|   |   s = "b"
|   echo(s)
|   return 0
