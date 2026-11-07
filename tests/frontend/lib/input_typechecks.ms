// expect: ok
// input() возвращает string, без stdin запускать нельзя, поэтому только проверка типов
[i32]main()
|   string s = input()
|   echo("got " + s)
|   return 0
