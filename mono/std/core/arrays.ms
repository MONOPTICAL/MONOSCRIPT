
// Функции для swap в массивах
[result<void>]swapInArray<T, Q>(array<Q>: arr, T: i, T: j)
|   // Потому что у нас тут д0хуя питон, то литералы передаются по значению, а не по ссылке
|   // Поэтому н1хуя через swap обычные переменные не сработают
|   // Нужно менять элементы массива напрямую который кстати вот приколь передаётся по ссылке
|   Q tmp = arr[i]
|   arr[i] = arr[j]
|   arr[j] = tmp
|   return ok()

// Функция сортировки массива
[result<array<T>>]sort<T>(array<T>: arr)
|   // Обычный бубл сорт ну и хух с ним, потом заменю на что-то более нормальное
|   i32 n = len(arr)
|   
|   if len(arr) <= 1 // Если элементов нет или только один, то пусть не вы3бывается
|   |   return ok(arr)
|   
|   for i in range(n)
|   |   for j in range(i, n)
|   |   |   
|   |   |   attempt
|   |   |   |   if arr[i] > arr[j]
|   |   |   |   |   swapInArray(arr, i, j)?
|   |   |   panic info
|   |   |   |   return err(info.message)
|   |   | 
|   |
|   // Чтобы нормальным людям было понятно наху я этот attempt кидаю
|   // Это потому что ху пойми что могут подсунуть в эту сортировку
|     
|   return ok(arr)

// Функция проверки наличия элемента в массиве, возвращает количество вхождений
[result<i32>]count<T>(array<T>: arr, T: value)
|   i32 result = 0
|   
|   if len(arr) == 0
|   |   return ok(0)
|   
|   for i in arr
|   |   attempt
|   |   |   if i == value
|   |   |   |   result = result + 1
|   |   panic info
|   |   |   return err(info.message)
|   |   
|   
|   return ok(result)

// Функция поиска индекса элемента в массиве, возвращает -1 если не найден
[result<i32>]indexOf<T>(array<T>: arr, T: value)
|   
|   for i in range(len(arr))
|   |   attempt
|   |   |   if arr[i] == value
|   |   |   |   return ok(i)
|   |   panic info
|   |   |   return err(info.message)
|   |   
|   
|   return ok(-1)

// Функция отображения элементов массива через заданную функцию, возвращает новый массив
[result<array<Q>>]mapped<T, Q>(array<T>: arr, func: f(T: x) -> Q)
|   array<Q> result = []
|   
|   for i in arr
|   |   attempt
|   |   |   push(result, f(i))
|   |   panic info
|   |   |   return err(info.message)
|   |
|   
|   return ok(result)
