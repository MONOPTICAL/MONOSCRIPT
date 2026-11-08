// expect: ok
// input() возвращает result<string>, без stdin запускать нельзя, поэтому только проверка типов
[i32]main()
|   result<string> r = input()
|   string s = r.value
|   echo("got " + s)
|   return 0
