use 
|> collections

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
|   // Функция очистки карты
|   [void]clear<T, Q>(map<T, Q>: m)
|   |   for k in keys(m)
|   |   |   del(m, k) 
|   
|   // Функция получения значения по ключу с возможностью указания значения по умолчанию
|   [Q]getOr<T,Q>(map<T, Q>: m, T: key, Q: defaultValue)
|   |   if has(m, key)
|   |   |   return m[key]
|   |   return defaultValue
|   
|   // Функция получения значения по ключу с вставкой значения по умолчанию, если ключ отсутствует
|   [Q]getOrInsert<T, Q>(map<T, Q>: m, T: key, Q: defaultValue)
|   |   if has(m, key)
|   |   |   return m[key]  
|   |   m[key] = defaultValue
|   |   return defaultValue
|   
|   // Функция обхода всех элементов карты с применением заданной функции
|   [void]forEach<T, Q>(map<T, Q>: m, func: f(T: key, Q: value)->void)
|   |   for k in m
|   |   |   f(k, m[k])
|   
|   // Функция получения всех пар ключ-значение карты
|   [array<pair<T, Q>>]entries<T, Q>(map<T, Q>: m)
|   |   array<pair<T, Q>> result = []
|   |   for k in m
|   |   |   pair<T, Q> p = pair(k, m[k])
|   |   |   push(result, p)
|   |   return result
|   
|   // Функция создания копии карты
|   [map<T, Q>]copy<T, Q>(map<T, Q>: m)
|   |   map<T, Q> result = {}
|   |   for k in m
|   |   |   result[k] = m[k]
|   |   return result
