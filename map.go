// Статическое объявление переменной
i32 x = 10
float pi = 3.14
string name = "test"
bool isReady = true

// Динамическое объявление переменной
x ^= 42
pi ^= 2.718
name ^= "example"
isReady ^= false

// Константы
const i32 port = 8080
const bool flag = true
final value ^= 99
final status ^= false // final поправить

// Сложные выражения
result ^= (x + 2) * (5 - 1)
message ^= (name + ": " + "OK")
sum ^= (pi + 1.0 + 2.0)
//combo ^= !isReady and (x > 0 or flag) <-- исправить надо

// Индексация в пизде
//arr ^= [1, 2, 3] <-- исправить надо
//arr[0] ^= 100	<-- исправить надо
//matrix[1][2] ^= x + 5	<-- исправить надо

// Вызов функции в присваивании
response ^= getData("url", 5)
nested ^= process(parse(data), x)