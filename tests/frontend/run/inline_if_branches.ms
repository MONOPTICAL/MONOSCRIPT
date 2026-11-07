// expect-output: 10
// expect-output: 20
// expect-output: 30
// expect-output: 7
[i32]choose(i32: n)
|   if n < 0: return 10
|   else if n == 0: return 20
|   else
|   |   return 30
[i32]main()
|   echo("" + choose(-1))
|   echo("" + choose(0))
|   echo("" + choose(1))
|   final i32 value
|   if true
|   |   if false: value = 9
|   |   else: value = 7
|   else: value = 8
|   echo("" + value)
|   for i in range(4)
|   |   if i == 0: continue
|   |   if i == 1: break
|   |   echo("BAD")
|   return 0
