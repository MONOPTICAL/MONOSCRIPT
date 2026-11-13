// expect-error: in generic function less with T = P, called at line 10
// Тип не подходит телу: ошибка говорит, с каким T и откуда вызвали
[struct]P
|   i32 v = 1

[i1]less<T>(T: a, T: b)
|   return a < b

[i32]main()
|   less(P(), P())
|   return 0
