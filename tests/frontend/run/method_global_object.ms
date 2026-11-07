// expect-output: 2
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

P g = P()

[i32]main()
|   g.birthday()
|   echo(cstr(g.age))
|   return 0
