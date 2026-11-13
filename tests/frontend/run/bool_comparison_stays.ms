// expect-output: yes
// expect-output: no
// Сравнения и логика остаются на i1
[i32]main()
|   x ^= 1
|   y ^= 0
|   if x and !y
|   |   echo("yes")
|   if x == y
|   |   echo("BAD")
|   else
|   |   echo("no")
|   return 0
