// expect-output: 6
[i32]sum(i32: n)
|   i32 r = 0
|   while n > 0
|   |   r = r + n
|   |   n = n - 1
|   return r

[i32]main()
|   echo(toString_int(sum(3)))
|   return 0
