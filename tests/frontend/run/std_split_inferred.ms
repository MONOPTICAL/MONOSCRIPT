// expect-output: y
// Тип результата читается из mono.toml, поэтому индексация работает и через ^=
[i32]main()
|   parts ^= split("x y", " ")
|   echo(parts[1])
|   return 0
