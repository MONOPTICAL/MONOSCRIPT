// expect-output: x:1
[struct]P
|   i32 age = 1
|   string name = "x"
|   [i32]getAge()
|   |   return self.age
|   [void]setAge(i32: a)
|   |   self.age = a
|   [void]birthday()
|   |   self.age = self.age + 1
|   [string]show() @pure
|   |   return self.name + ":" + self.age

[string]f(P: p) @pure
|   return p.show()

[i32]main()
|   echo(f(P()))
|   return 0
