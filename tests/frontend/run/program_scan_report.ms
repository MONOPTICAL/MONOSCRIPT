// expect-output: a.local -> 90
// expect-output: b.local -> 30
// expect-output: c.local -> 90
// expect-output: d.local -> 0
// expect-output: tag remote: 2
// expect-output: tag admin: 2
// expect-output: tag web: 1
// expect-output: seen 4, total 210
// expect-output: orig 90, clone 0
// expect-output: hosts 3
// Настоящая программа на всё сразу: 512 тестов по отдельности не заметили, что она падает
[struct]Finding
|   string host = "?"
|   i32 port = 0
|   i32 score = 0
|   array<string> tags = []

i32 seen = 0

[Finding]scan(string: host, i32: port)
|   seen = seen + 1
|   Finding f = Finding()
|   f.host = host
|   f.port = port
|   if port == 22 or port == 3389
|   |   f.score = 90
|   |   f.tags = ["remote", "admin"]
|   else if port == 80
|   |   f.score = 30
|   |   f.tags = ["web"]
|   return f

[i64]total(array<Finding>: list) @pure
|   i64 s = 0
|   for f in list
|   |   s = s + f.score
|   return s

[i32]main()
|   array<Finding> found = [scan("a.local", 22), scan("b.local", 80), scan("c.local", 3389), scan("d.local", 8080)]
|   map<string,i32> byHost = {}
|   map<string,i32> tagCount = {}
|   for f in found
|   |   byHost[f.host] = f.score
|   |   for t in f.tags
|   |   |   if has(tagCount, t)
|   |   |   |   tagCount[t] = tagCount[t] + 1
|   |   |   else
|   |   |   |   tagCount[t] = 1
|   for h in byHost
|   |   echo(h + " -> " + byHost[h])
|   for t in tagCount
|   |   echo("tag " + t + ": " + tagCount[t])
|   echo("seen " + seen + ", total " + total(found))
|   Finding clone = copy(found[0])
|   clone.score = 0
|   echo("orig " + found[0].score + ", clone " + clone.score)
|   del(byHost, "d.local")
|   echo("hosts " + len(byHost))
|   return 0
