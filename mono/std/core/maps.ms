[namespace]maps
|   // Функция получения всех ключей карты
|   [array<T>]keys<T, Q>(map<T, Q>: m)
|   |   array<T> result = []
|   |   for k in m
|   |   |   push(result, k)
|   |   return result
|   
|   // Функция получения всех значений карты
|   [array<Q>]values<T, Q>(map<T, Q>: m)
|   |   array<Q> result = []
|   |   for k in m
|   |   |   push(result, m[k])
|   |   return result
|   
