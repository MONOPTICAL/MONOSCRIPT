// expect-runtime-error: index 100 out of bounds (len 3) at line 5
// Проверка не должна читать дальше нуля
[i32]main()
|   string s = "abc"
|   echo(cstr(s[100]))
|   return 0
