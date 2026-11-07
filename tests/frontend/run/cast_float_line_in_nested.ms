// expect-runtime-error: float value out of range for i8 at line 6
// В условии внутри цикла номер строки свой
[i32]main()
|   float a = 200.0
|   while 1 == 1
|   |   i8 x = a->i8
|   |   break
|   return 0
