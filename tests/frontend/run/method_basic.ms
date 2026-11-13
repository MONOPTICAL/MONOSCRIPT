// expect-output: 1
// expect-output: 7
// expect-output: 8
// expect-output: x:8
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

[i32]main()
|   P p = P()
|   echo(cstr(p.getAge()))
|   p.setAge(7)
|   echo(cstr(p.getAge()))
|   p.birthday()
|   echo(cstr(p.age))
|   echo(p.show())
|   return 0
