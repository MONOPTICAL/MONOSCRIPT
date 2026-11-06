// expect-output: 7
// Копия поверхностная: вложенные массивы общие
[i32]main()
|   array<array<i32>> a = [[1]]
|   array<array<i32>> b = copy(a)
|   b[0][0] = 7
|   echo(toString_int(a[0][0]))
|   return 0
