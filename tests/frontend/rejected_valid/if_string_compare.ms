// expect: ok
// xfail: для строк нельзя вывести числовой тип при касте условия, Statements.cpp:450
[i32]main()
|   string s = "a"
|   if s == "a"
|   |   echo("y")
|   return 0
