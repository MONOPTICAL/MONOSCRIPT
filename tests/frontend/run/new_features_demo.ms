// expect-output: 192.168.1.1:8080
// expect-output: 443
// expect-output: 1024 8 16 -1
// expect-output: 200 44 255
// expect-output: 10 метод 10 GET
// expect-output: число 42, строка hi
// expect-output: b 1
// expect-output: https=443
// expect-output: 3
// expect-output: 6
// expect-output: 9
// expect-output: 12
// expect-output: 4
// expect-output: 2 start
use
|> mono.core.arrays

// ---------- namespace ----------
[namespace] net
|   u16 defaultPort = 8080
|
|   [struct]Addr
|   |   u32 ip = 0
|   |   u16 port = 0
|
|   [Addr]make(u32: ip, u16: port)
|   |   Addr a = Addr()
|   |   a.ip = ip
|   |   a.port = port
|   |   return a
|
|   [string]octet(u32: ip, u32: shift)
|   |   return cstr(((ip shr shift) band 255)->u8)
|
|   [string]show(Addr: a)
|   |   return octet(a.ip, 24) + "." + octet(a.ip, 16) + "." + octet(a.ip, 8) + "." + octet(a.ip, 0) + ":" + cstr(a.port)

// ---------- enum ----------
[enum]Method
|   Get
|   Post
|   Delete = 10

// ---------- generic struct ----------
[struct]Stack<T>
|   array<T> items = []
|   [void]push(T: x)
|   |   push(self.items, x)
|   [T]pop()
|   |   return pop(self.items)
|   [i32]size()
|   |   return len(self.items)

[struct]Pair<K, V>
|   K key
|   V value

// ---------- overloading ----------
[string]describe(i32: x)
|   return "число " + x

[string]describe(string: s)
|   return "строка " + s

[string]describe(Method: m)
|   if m == Method.Get
|   |   return "GET"
|   return "метод " + cstr(m)

[i32]main()
|   // namespace + unsigned + bitwise
|   net.Addr a = net.make(3232235777, net.defaultPort)
|   echo(net.show(a))
|   net.defaultPort = 443
|   echo(cstr(net.defaultPort))
|
|   // ** и битовые
|   echo(cstr(2 ** 10) + " " + cstr(12 band 10) + " " + cstr(1 shl 4) + " " + cstr(bnot 0))
|
|   // unsigned: деление, сравнение, каст
|   u8 b = 200
|   u8 wrapped = b + 100
|   echo(cstr(b) + " " + cstr(wrapped) + " " + cstr((0 - 1)->u8))
|
|   // enum
|   Method m = Method.Delete
|   echo(cstr(m) + " " + describe(m) + " " + describe(Method.Get))
|
|   // overloading
|   echo(describe(42) + ", " + describe("hi"))
|
|   // generic structs
|   Stack<string> s = Stack()
|   s.push("a")
|   s.push("b")
|   echo(s.pop() + " " + cstr(s.size()))
|   Pair<string, u16> p = Pair()
|   p.key = "https"
|   p.value = 443
|   echo(p.key + "=" + cstr(p.value))
|
|   // lambdas + closures
|   i32 k = 3
|   array<i32> nums = [1, 2, 3, 4]
|   for x in arrays.mapped(nums, [i32](i32: v) -> v * k)
|   |   echo(cstr(x))
|   func<i32, i32> addK = [i32](i32: v) -> v + k
|   k = 100
|   echo(cstr(addK(1)))
|   array<string> log = []
|   note ^= [i32](string: msg):
|   |   push(log, msg)
|   |   return len(log)
|   note("start")
|   echo(cstr(note("end")) + " " + log[0])
|   return 0
