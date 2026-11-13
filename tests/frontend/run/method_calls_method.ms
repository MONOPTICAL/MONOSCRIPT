// expect-output: 3
[struct]C
|   i32 n = 0
|   [void]inc()
|   |   self.n = self.n + 1
|   [void]twice()
|   |   self.inc()
|   |   self.inc()
|   [i32]thrice()
|   |   self.twice()
|   |   self.inc()
|   |   return self.n

[i32]main()
|   C c = C()
|   echo(cstr(c.thrice()))
|   return 0
