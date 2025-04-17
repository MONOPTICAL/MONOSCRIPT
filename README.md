# MONOSCRIPT (В разработке)

**MONOSCRIPT** — это легкий, читаемый язык программирования, вдохновленный Python и C-подобным синтаксисом. Он предназначен для написания безопасных, модульных скриптов с фокусом на простоту, читаемость и поддержку плагинов. 

---

## 🧠 Особенности языка

### ✅ Объявление функций
**Синтаксис:** `[тип]имяФункции(тип: аргумент, ...)`
```cpp
[void]main()
|   echo("Привет, мир!")
```
- Тип указывается в скобках `[]`
- Аргументы указываются как `тип: имя`
- Отступы обозначаются с помощью символа `|`

### ✅ Вложенные функции
**Синтаксис:** объявление функции внутри другой
```cpp
[void]main()
|   [i32]sum(i32: a, i32: b)
|   |   return a + b
```

### ✅ Объявление переменных
```cpp
Тип имя = значение       // Статическая инициализация
имя ^= значение          // Динамическая (auto) ининициализация
```

### ✅ Константы
```cpp
const Тип имя = значение   // Статическая константа
final имя ^= значение      // Динамическая константа
```

**Разница:**
- `const` — значение и тип заданы явно и не могут быть изменены.
- `final` — значение присваивается один раз, но тип может быть выведен.

### ✅ Операции и выражения
```cpp
result ^= !(x > 5 and y < 10)
```
- Поддержка всех базовых операторов (`+`, `-`, `*`, `/`, `==`, `!=`, `<`, `>`, `<=`, `>=`)
- Логические операторы: `and`, `or`, `!`

### ✅ Циклы и условия
**Условия:**
```cpp
if выражение
|   блок
else if выражение
|   блок
else
|   блок
```

**Циклы:**
```cpp
for переменная in выражение
|   блок
```

### ✅ Массивы и словари
```go
array<i32> числа ^= [1, 2, 3]
map<string, i32> очки ^= {"Alice": 10, "Bob": 8}
```
- Доступ через индекс: `массив[0]`, `словарь["ключ"]`

### ✅ Структуры и методы
```cpp
[struct]User
|  map<string,string> User_Info
|  [User]__init__(string: name, string: password, string: id)
|  |  User_Info = {"name" : name, "password": password, "id" : id} 
|
|  [map<string,string>]GetUserInfo()
|  |  return User_Info   
```

**Альтернативная реализация методов:**
```cpp
[void]Logger::logInfo()
|   echo("Логирование")
```
или в самом классе:
```cpp
[void]logInfo()
|   echo("Логирование")
```

### ✅ Возврат значений
```cpp
return выражение
```

### ✅ Передача параметров
- Примитивы (`i1`, `i8`, `i32`, `i64`, `float`, `string`, `bool`) передаются **по значению**.
- Структуры данных (`map`, `array`, `struct`) передаются **по ссылке**.
- Поведение полностью аналогично **Python**.

---

## 🔤 Ключевые слова
```
if, else, for, while, return, break, continue,
true, false, null, none, import, const, 
final, in, and, or, is, public, private, define
```

## 🔢 Встроенные типы
```
i1, i8, i32, i64, string, void, array, map, float, struct, class
```

---

## 🛠 Пример полной программы
```cpp
[void]bubbleSort(array<i32>: arr)
|   n ^= len(arr)
|   for i in range(0, n)
|   |   for j in range(0, n - 1)
|   |   |   if arr[j] > arr[j + 1]
|   |   |   |   temp ^= arr[j]
|   |   |   |   arr[j] = arr[j + 1]
|   |   |   |   arr[j + 1] = temp

[void]printArray(array<i32>: arr)
|   for i in range(0, len(arr))
|   |   echo(arr[i])

[void]main()
|   array<i32> nums = [5, 3, 1, 4, 2]
|   echo("Before sort:")
|   printArray(nums)
|
|   bubbleSort(nums)
|
|   echo("After sort:")
|   printArray(nums)
```

---

## 🛡️ LGPL-2.1 — свободная лицензия

Этот проект распространяется под лицензией **LGPL-2.1 (GNU Lesser General Public License)**. Это означает:

- Вы можете **свободно использовать, модифицировать и распространять** MONOSCRIPT.
- Изменения самого интерпретатора должны быть также открыты, если распространяются.
- Встраивание MONOSCRIPT в другие приложения **не требует** открытия их исходников.

---

Сделано с ❤️ для разработчиков.

