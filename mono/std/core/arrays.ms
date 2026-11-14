[namespace]arrays
|   // Функции для swap в массивах
|   [void]swapInArray<T, Q>(array<Q>: arr, T: i, T: j)
|   |   // Потому что у нас тут д0хуя питон, то литералы передаются по значению, а не по ссылке
|   |   // Поэтому н1хуя через swap обычные переменные не сработают
|   |   // Нужно менять элементы массива напрямую который кстати вот приколь передаётся по ссылке
|   |   Q tmp = arr[i]
|   |   arr[i] = arr[j]
|   |   arr[j] = tmp
|   // Функция сортировки массива
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
|   |   // Чтобы нормальным людям было понятно наху я этот attempt кидаю
|   |   // Это потому что ху пойми что могут подсунуть в эту сортировку
|   |     
|   |   return arr
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
|   // Функция поиска индекса элемента в массиве, возвращает -1 если не найден
|   [i32]indexOf<T>(array<T>: arr, T: value)
|   |   
|   |   for i in range(len(arr))
|   |   |   if arr[i] == value
|   |   |   |   return i
|   |   |   
|   |   
|   |   return -1
|   // Функция отображения элементов массива через заданную функцию, возвращает новый массив
|   [array<Q>]mapped<T, Q>(array<T>: arr, func: f(T: x) -> Q)
|   |   array<Q> result = []
|   |   
|   |   for i in arr
|   |   |   push(result, f(i))
|   |   |
|   |   
|   |   return result
|   