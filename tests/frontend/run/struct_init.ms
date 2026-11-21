// expect-output: 127.0.0.1:80
// expect-output: 0.0.0.0:0
// expect-output: 3
// expect-output: http 8080
// expect-output: 7
// expect-output: 2
[struct]Addr
|   string host = "0.0.0.0"
|   u16 port = 0
|   [void]init(string: host, u16: port)
|   |   self.host = host
|   |   self.port = port
|   [string]show()
|   |   return self.host + ":" + cstr(self.port)

[struct]Counter
|   i32 n = 0
|   [void]init()
|   |   self.n = 3

[struct]Pair<K, V>
|   K key
|   V value
|   [void]init(K: k, V: v)
|   |   self.key = k
|   |   self.value = v

[struct]Box
|   Addr inner = Addr("x", 7)

[Addr]local(u16: p)
|   return Addr("127.0.0.1", p)

[i32]main()
|   echo(local(80).show())
|   Addr d = Addr()
|   echo(d.show())
|   echo(cstr(Counter().n))
|   Pair<string, u16> p = Pair("http", 8080)
|   echo(p.key + " " + cstr(p.value))
|   echo(cstr(Box().inner.port))
|   array<Addr> list = [Addr("a", 1), Addr("b", 2)]
|   echo(cstr(list[1].port))
|   return 0
