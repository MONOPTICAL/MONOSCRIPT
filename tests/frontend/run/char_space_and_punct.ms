// expect-output: [ ]
// expect-output: [,]
// expect-output: [(]
// expect-output: [/]
// Символы, которые лексер в обычном коде понимает по-своему
[i32]main()
|   echo("[" + ' ' + "]")
|   echo("[" + ',' + "]")
|   echo("[" + '(' + "]")
|   echo("[" + '/' + "]")
|   return 0
