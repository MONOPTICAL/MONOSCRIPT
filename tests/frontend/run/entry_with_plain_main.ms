// expect-output: from main
// expect-output: in foo
// Обычная функция main рядом с @entry не конфликтует со стартовой
[i32]main()
|   echo("from main")
|   return 0

[i32]foo() @entry
|   main()
|   echo("in foo")
|   return 0
