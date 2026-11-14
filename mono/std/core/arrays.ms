[namespace]arrays
|   // Функции для swap в массивах
|   [void]swapInArray<T, Q>(array<Q>: arr, T: i, T: j)
|   |   // Потому что у нас тут д0хуя питон, то литералы передаются по значению, а не по ссылке
|   |   // Поэтому н1хуя через swap обычные переменные не сработают
|   |   // Нужно менять элементы массива напрямую который кстати вот приколь передаётся по ссылке
|   |   Q tmp = arr[i]
|   |   arr[i] = arr[j]
|   |   arr[j] = tmp
|   
|   // Функция сортировки массива
|   
|   [array<T>]sort<T>(array<T>: arr)
|   |   // Обычный бубл сорт ну и хух с ним, потом заменю на что-то более нормальное
|   |   i32 n = len(arr)
|   |   
|   |   if len(arr) <= 1 // Если элементов нет или только один, то пусть не вы3бывается
|   |   |   return arr
|   |   
|   |   for i in range(n)
|   |   |   for j in range(i, n)
|   |   |   |   
|   |   |   |   if arr[i] > arr[j]
|   |   |   |   |   swapInArray(arr, i, j)
|   |   |   | 
|   |   |
|   |     
|   |   return arr
|   
|   // Функция проверки наличия элемента в массиве, возвращает количество вхождений
|   [i32]count<T>(array<T>: arr, T: value)
|   |   i32 result = 0
|   |   
|   |   if len(arr) == 0
|   |   |   return 0
|   |   
|   |   for i in arr
|   |   |   if i == value
|   |   |   |   result = result + 1
|   |   |   
|   |   
|   |   return result
|   
|   // Функция поиска индекса элемента в массиве, возвращает -1 если не найден
|   [i32]indexOf<T>(array<T>: arr, T: value)
|   |   
|   |   for i in range(len(arr))
|   |   |   if arr[i] == value
|   |   |   |   return i
|   |   |   
|   |   
|   |   return -1
|   
|   // Функция отображения элементов массива через заданную функцию, возвращает новый массив
|   [array<Q>]mapped<T, Q>(array<T>: arr, func: f(T: x) -> Q)
|   |   array<Q> result = []
|   |   for i in arr
|   |   |   push(result, f(i))
|   |   
|   |   return result
|   
|   // Функция фильтрации: оставляет элементы, для которых f вернула true
|   [array<T>]filter<T>(array<T>: arr, func: f(T: x) -> i1)
|   |   array<T> result = []
|   |   for i in arr
|   |   |   if f(i)
|   |   |   |   push(result, i)
|   |   
|   |   return result
|   
|   // Функция реверса массива, изменяет элементы оригинального массива
|   [void]reverse<T>(array<T>: arr)
|   |   i32 n = len(arr)
|   |   for i in range(n / 2)
|   |   |   swapInArray(arr, i, n - i - 1)
|   
|   // Функция получения реверсированного массива, не изменяет оригинальный массив
|   [array<T>]reversed<T>(array<T>: arr)
|   |   array<T> result = []
|   |   for i in range(len(arr))
|   |   |   push(result, arr[len(arr) - 1 - i])
|   |   
|   |   return result
|   
|   // Возвращает копию элементов [start, end); неверные границы дают пустой массив
|   [array<T>]slice<T>(array<T>: arr, i32: start, i32: end)
|   |   array<T> result = []
|   |   if start >= end or start < 0 or end > len(arr)
|   |   |   return result
|   |   for i in range(start, end)
|   |   |   push(result, arr[i])
|   |   
|   |   return result
|   
|   [i1]contains<T>(array<T>: arr, T: element)
|   |   for i in arr
|   |   |   if i == element
|   |   |   |   return true
|   |   
|   |   return false
|   
|   // Максимальный элемент; на пустом массиве паника выхода за границы
|   [T]max<T>(array<T>: arr)
|   |   T result = arr[0]
|   |   for i in arr
|   |   |   if i > result
|   |   |   |   result = i
|   |   |   
|   |   
|   |   return result
|   
|   // Минимальный элемент; на пустом массиве паника выхода за границы
|   [T]min<T>(array<T>: arr)
|   |   T result = arr[0]
|   |   for i in arr
|   |   |   if i < result
|   |   |   |   result = i
|   |   |   
|   |   
|   |   return result
|   
|   // Удаление элемента по индексу; неверный индекс даёт панику выхода за границы
|   [void]removeAt<T>(array<T>: arr, i32: index)
|   |   arr[index] = arr[index] // проверка границ до изменения массива
|   |   for i in range(index, len(arr) - 1)
|   |   |   arr[i] = arr[i + 1]
|   |   
|   |   pop(arr)
|   
|   // Вставка элемента по индексу (index == len добавляет в конец); иначе паника выхода за границы
|   [void]insert<T>(array<T>: arr, i32: index, T: element)
|   |   if index != len(arr)
|   |   |   arr[index] = arr[index] // проверка границ до push, чтобы не испортить массив
|   |   push(arr, element)
|   |   for k in range(len(arr) - 1 - index)
|   |   |   i32 i = len(arr) - 1 - k
|   |   |   arr[i] = arr[i - 1]
|   |   
|   |   arr[index] = element
|   
|   // Сумма элементов, пустой массив даёт 0
|   [i64]sum<T>(array<T>: arr)
|   |   i64 result = 0
|   |   for i in arr
|   |   |   result = result + i
|   |   
|   |   return result
