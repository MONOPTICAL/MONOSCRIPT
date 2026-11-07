// expect-output: 5
// expect-output: 6
// Строки сравниваются по содержимому, а не по адресу
[i32]main()
|   map<string,i32> m = {"ab": 5}
|   echo(toString_int(m["a" + "b"]))
|   string k = "x"
|   m[k + 1] = 6
|   echo(toString_int(m["x1"]))
|   return 0
