[void]main()
|   const map<string, i32> roles = {"admin": 1, "user": 0}
|   final array<map<string, i32>> accessList ^= []

|   // Struct definition
|   [struct] User
|   | string : name
|   | i32 : age
|   | bool : isActive

|   // Class definition with access specifiers
|   [class] Logger
|   | public:
|   | | void : logInfo
|   | private:
|   | | string : secretKey

|   [bool]isAdmin(string: role)
|   |   return role == "admin"

|   [i32]calcScore(i32: a, b)
|   |   result ^= (a * 2 + b) / (a + 1)
|   |   return result

|   users ^= ["Alice", "Bob", "Eve"]
|   for i in range(0, len(users))
|   |   name ^= users[i]
|   |   isPrivileged ^= isAdmin("admin") or name == "Eve"
|   |   score ^= calcScore(i, len(name))
|   |   echo("User: " + name + ", Score: " + score)
|   |   map<string, i32> entry ^= {"index": i, "score": score}
|   |   accessList ^= accessList + [entry]

|   totalScore ^= 0
|   for i in range(0, len(accessList))
|   |   totalScore ^= totalScore + accessList[i]["score"]

|   echo("Total score: " + totalScore)

|   if totalScore > 10 and !(totalScore == 15)
|   |   echo("Above threshold")
|   else if totalScore == 15
|   |   echo("Exact match")
|   else
|   |   echo("Below threshold")

|   done ^= true
|   echo("Done: " + done)

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

// Класс Map<K, V>
[class]<Map<K, V>>()
|   keys ^= []           // array<K>
|   values ^= []         // array<V>

|   [void]put(K: key, V: value)
|   |for i in range(0, len(keys))
|   |   |   if keys[i] == key
|   |   |   |   values[i] ^= value
|   |   |   |   return
|   |   keys.append(key)
|   |   values.append(value)

|   [V]get(K: key)
|   |   for i in range(0, len(keys))
|   |   |   if keys[i] == key
|   |   |   |   return values[i]
|   |   return null

|   [bool]has(K: key)
|   |   for i in range(0, len(keys))
|   |   |   if keys[i] == key
|   |   |   |   return true
|   |   return false

|   [void]remove(K: key)
|   |   for i in range(0, len(keys))
|   |   |   if keys[i] == key
|   |   |   |   keys.removeAt(i)
|   |   |   |   values.removeAt(i)
|   |   |   |   return

[void]main()
|   maap ^= map<string, i32>()
|   maap.put("a", 10)
|   maap.put("b", 20)
|   maap.put("a", 30)         // обновит значение "a"

|   echo("a = " + maap.get("a"))   // "a = 30"
|   echo("b = " + maap.get("b"))   // "b = 20"
|   echo("has c? " + maap.has("c")) // "false"

|   maap.remove("a")
|   echo("has a? " + maap.has("a")) // "false"