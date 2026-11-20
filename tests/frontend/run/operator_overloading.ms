// expect-output: 4 6
// expect-output: 1.5 2.5
// expect-output: 6 10
// expect-output: 2 4
// expect-output: equal
// expect-output: 7 12
[struct]Vec<T>
|   T x
|   T y
|   [void]init(T: x, T: y)
|   |   self.x = x
|   |   self.y = y
|   [Vec<T>]operator+(Vec<T>: other) @pure
|   |   return Vec(self.x + other.x, self.y + other.y)
|   [Vec<T>]operator-(Vec<T>: other) @pure
|   |   return Vec(self.x - other.x, self.y - other.y)
|   [Vec<T>]operator*(T: k) @pure
|   |   return Vec(self.x * k, self.y * k)
|   [Vec<T>]operator*(Vec<T>: other) @pure
|   |   return Vec(self.x * other.x, self.y * other.y)
|   [i1]operator==(Vec<T>: other) @pure
|   |   return self.x == other.x and self.y == other.y
|   [string]str() @pure
|   |   return cstr(self.x) + " " + cstr(self.y)

[i32]main()
|   Vec<i32> a = Vec(1, 2)
|   Vec<i32> b = Vec(3, 4)
|   Vec<i32> sum = a + b
|   echo(sum.str())
|   Vec<f64> f = Vec(0.5, 1.0)
|   Vec<f64> g = Vec(1.0, 1.5)
|   Vec<f64> fsum = f + g
|   echo(fsum.str())
|   Vec<i32> c = b * 2 + a * a - a
|   echo(c.str())
|   Vec<i32> d = a * 3 - a
|   echo(d.str())
|   Vec<i32> expected = Vec(4, 6)
|   if a + b == expected and !(a == b)
|   |   echo("equal")
|   Vec<i32> e = a + b + a * 2 + a * 0 + a
|   echo(e.str())
|   return 0
