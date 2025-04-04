# MONOSCRIPT — Спецификация Языка

MONOSCRIPT — это безопасный, минималистичный, читаемый язык плагинов для системы MONOPTICAL. Он предназначен для написания скриптов, выполняющих сканирование и анализ поведения pod'ов внутри Kubernetes-кластера.

## ✨ Общие особенности
- Минималистичный синтаксис
- Типизация переменных и аргументов
- Интерпретируемый AST с sandbox-исполнением
- Безопасный API-доступ к среде (pod, net, alert, log и т.д.)
- Возможность модульности через `import`

---

## ✍Основной синтаксис

### Объявление переменных
```mscript
a ^= 5             # Динамическая типизация
i32 b = 10         # Статическая типизация(Явная типизация)
```

### Функции
```mscript
[i32]sum(i32 a, i32 b)
|   return a + b

[void]main()
|   result ^= sum(3, 5)
|   echo(result)
```

### Вызов функции
```mscript
echo(sum(3, 5))
```

### Условные операторы
```mscript
if scan_result == false
|   alert("vuln detected")
else
|   echo("clean")
```

### Циклы
```mscript
for i in range(0, 10)
|   echo(i)

while is_running
|   wait(1000)
```

### Совпадения (match)
```mscript
match status
|   case "OK": echo("OK")
|   case "FAIL": alert("Fail")
```

### Импорт
```mscript
import "stdlib/net"
import "plugins/myplugin"
```

### Ошибки
```mscript
try
|   connectSock = net.connect(ip, port)
|   if connectSock == null
|   |   throw "no conn"
catch e
|   alert("Scan failed: " + e)
```

---

## ⚖️ Типы
- `i32` / `i64` — числа
- `string` — строки
- `bool` — `true` / `false`
- `array<T>` — массивы
- `null` — специальное пустое значение

---

## ⚙️ Встроенные функции (API)
- `echo(value)`
- `alert(msg)`
- `log(msg)`
- `net.connect(ip, port)` → сокет или null
- `get_pod(name)` → pod info
- `open_ports(pod)` → array<i32>
- `contains(array, value)` → bool
- `range(a, b)` → array<i32>
- `wait(ms)` — пауза

---

## ⚠️ Ограничения и защита
- Запрет доступа к `system()`, `os`, `file`, `socket`
- Таймаут на выполнение (время/инструкции)
- Только безопасные API
- Ограниченное поле зрения (sandbox)

---

## 📄 Пример сканера
```mscript
[bool]customScan(string IP, i32 Port)
|   sock ^= net.connect(IP, Port)
|   return sock == null

[void]main(i32 argc, array<string> argv)
|   result ^= customScan(argv[0], argv[1])
|   echo(result)
```

---

## 🔒 Цели дизайна
- Сделать написание плагинов безопасным, легким и читаемым
- Убрать риск повреждения системы через плагин
