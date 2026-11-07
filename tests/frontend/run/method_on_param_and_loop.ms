// expect-output: 2
// expect-output: 3
// expect-output: 3
// Метод меняет тот же объект: это ссылка
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

[void]grow(P: x)
|   x.birthday()

[i32]main()
|   P p = P()
|   grow(p)
|   echo(cstr(p.age))
|   array<P> ps = [P(), P()]
|   for q in ps
|   |   q.setAge(3)
|   echo(cstr(ps[0].getAge()))
|   echo(cstr(ps[1].getAge()))
|   return 0
