// expect-output: 4
// expect-output: 1
// expect-output: hi
// expect-output: [] 0
// expect-output: http 80
// expect-output: 200
// expect-output: 3
// expect-output: 9
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

[i32]main()
|   Stack<i32> s = Stack()
|   s.push(3)
|   s.push(4)
|   echo(cstr(s.pop()))
|   echo(cstr(s.size()))
|   Stack<string> w = Stack()
|   w.push("hi")
|   echo(w.pop())
|   Pair<string, u16> p = Pair()
|   echo("[" + p.key + "] " + cstr(p.value))
|   p.key = "http"
|   p.value = 80
|   echo(p.key + " " + cstr(p.value))
|   Pair<string, Stack<u8>> q = Pair()
|   q.value.push(200)
|   echo(cstr(q.value.pop()))
|   echo(cstr(make().value + 3))
|   array<Pair<string, u16>> list = []
|   push(list, p)
|   push(list, p)
|   echo(cstr(len(list) * 4 + 1))
|   return 0

[Pair<i32, i32>]make()
|   return Pair()
