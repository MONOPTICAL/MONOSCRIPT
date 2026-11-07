// expect-output: calls=0
// Правая часть не вычисляется, если ответ уже ясен. Считаем вызовы: лишнюю строку expect-output не заметит
i32 calls = 0

[i1]boom()
|   calls = calls + 1
|   return 1 == 1

[i32]main()
|   i32 j = 0
|   if j > 0 and boom()
|   |   calls = calls + 100
|   if j == 0 or boom()
|   |   j = 0
|   while j > 0 and boom()
|   |   j = j - 1
|   i1 r = j > 0 and boom()
|   array<i32> a = [1]
|   if j > 0 and a[j - 1] > 0
|   |   calls = calls + 100
|   echo("calls=" + calls)
|   return 0
