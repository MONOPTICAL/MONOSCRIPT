// expect-output: 4
[struct]Store
|   map<string, i32> items = {"a": 4}
|   [result<i32>]find(string: k) @pure
|   |   if has(self.items, k)
|   |   |   return ok(self.items[k])
|   |   return err("missing " + k)

[i32]main()
|   Store s = Store()
|   result<i32> r = s.find("a")
|   echo(cstr(r.value))
|   return 0
