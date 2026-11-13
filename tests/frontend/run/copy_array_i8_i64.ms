// expect-output: -2
// expect-output: 5000000000
// Размер элемента считается верно: скопирован весь блок
[i32]main()
|   array<i8> a = [1,-2]
|   array<i8> b = copy(a)
|   i32 x = b[1]
|   echo(toString_int(x))
|   array<i64> c = [1,5000000000]
|   array<i64> d = copy(c)
|   echo(toString_long(d[1]))
|   return 0
