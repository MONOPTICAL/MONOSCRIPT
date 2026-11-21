// expect-output: 15
// expect-output: 2.0
// expect-output: hello world
// expect-output: 7 30
// expect-output: 4 6
// expect-output: 8 12
[struct]Counter
|   i32 n = 5

[struct]Vec<T>
|   T x
|   T y
|   [void]init(T: x, T: y)
|   |   self.x = x
|   |   self.y = y
|   [Vec<T>]operator+(Vec<T>: other) @pure
|   |   return Vec(self.x + other.x, self.y + other.y)
|   [Vec<T>]operator*(T: k) @pure
|   |   return Vec(self.x * k, self.y * k)

[i32]main()
|   i32 i = 10
|   i += 2 * 3
|   i -= 1
|   i *= 2
|   i /= 3
|   i %= 100
|   i += 5
|   echo(cstr(i))
|   f64 f = 1.0
|   f *= 2.5
|   f -= 2.5
|   f += 2.0
|   echo(cstr(f))
|   string s = "hello"
|   s += " world"
|   echo(s)
|   Counter c = Counter()
|   c.n += 2
|   array<i32> a = [10, 20]
|   a[1] += a[0]
|   echo(cstr(c.n) + " " + cstr(a[1]))
|   Vec<i32> v = Vec(1, 2)
|   Vec<i32> w = Vec(3, 4)
|   v += w
|   echo(cstr(v.x) + " " + cstr(v.y))
|   v *= 2
|   echo(cstr(v.x) + " " + cstr(v.y))
|   return 0
