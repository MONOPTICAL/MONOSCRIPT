// expect-error:
// xfail: значение обрезается до int при проверке диапазона, Utils.cpp:231
[i32]main()
|   i8 a = 4294967296
|   return 0
