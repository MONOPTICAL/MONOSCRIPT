// expect-output: 5
// Результат int, его можно складывать и сравнивать
[i32]main()
|   i32 n = count("a-b-c", "-") + count("xx", "x") + count("q", "q")
|   echo(cstr(n))
|   return 0
