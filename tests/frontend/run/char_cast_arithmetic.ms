// expect-output: b
// expect-output: 3
// expect-output: A
// Символьная арифметика только через каст
[i32]main()
|   char c = 'a'
|   char d = (c->i32 + 1)->char
|   echo(cstr(d))
|   i32 digit = '3'->i32 - '0'->i32
|   echo(cstr(digit))
|   char up = (c->i32 - 32)->char
|   echo(cstr(up))
|   return 0
