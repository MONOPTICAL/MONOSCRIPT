// expect-output: 2
// Литерал в цикле каждый раз новый объект
[i32]main()
|   array<i32> first = [0]
|   array<i32> cur = [0]
|   for i in range(2)
|   |   array<i32> a = [1]
|   |   if i == 0
|   |   |   first = a
|   |   cur = a
|   cur[0] = 2
|   first[0] = 1
|   echo(toString_int(cur[0] + first[0] - 1))
|   return 0
