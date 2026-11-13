// expect-output: 2
// expect-output: 7
[i32]size<K, V>(map<K, V>: m)
|   return len(m)

[V]get<K, V>(map<K, V>: m, K: k)
|   return m[k]

[i32]main()
|   map<string,i32> m = {"a": 7, "b": 8}
|   echo(cstr(size(m)))
|   echo(cstr(get(m, "a")))
|   return 0
