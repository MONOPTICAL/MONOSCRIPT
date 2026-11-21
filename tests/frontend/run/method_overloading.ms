// expect-output: int 5
// expect-output: str a
// expect-output: f64 1.5
// expect-output: 127.0.0.1:80
// expect-output: 0.0.0.0:8080
// expect-output: 0.0.0.0:1
// expect-output: 3 x
[struct]Printer
|   [string]show(i32: x)
|   |   return "int " + x
|   [string]show(string: s)
|   |   return "str " + s
|   [string]show(f64: x)
|   |   return "f64 " + cstr(x)

[struct]Addr
|   string host = "0.0.0.0"
|   u16 port = 0
|   [void]init()
|   |   self.port = 1
|   [void]init(u16: port)
|   |   self.port = port
|   [void]init(string: host, u16: port)
|   |   self.host = host
|   |   self.port = port
|   [string]str()
|   |   return self.host + ":" + cstr(self.port)

[struct]Box<T>
|   T value
|   string tag = ""
|   [void]init(T: v)
|   |   self.value = v
|   [void]init(T: v, string: tag)
|   |   self.value = v
|   |   self.tag = tag

[i32]main()
|   Printer p = Printer()
|   echo(p.show(5))
|   echo(p.show("a"))
|   echo(p.show(1.5))
|   echo(Addr("127.0.0.1", 80).str())
|   u16 port = 8080
|   echo(Addr(port).str())
|   echo(Addr().str())
|   Box<i32> b = Box(3, "x")
|   echo(cstr(b.value) + " " + b.tag)
|   return 0
