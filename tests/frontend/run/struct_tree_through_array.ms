// expect-output: 2
// expect-output: 5
// Дерево без nullable: дети лежат в массиве, а пустой массив это не рекурсия
[struct]Node
|   i32 v = 0
|   array<Node> kids = []

[i32]sum(Node: n)
|   i32 s = n.v
|   for k in n.kids
|   |   s = s + sum(k)
|   return s

[i32]main()
|   Node a = Node()
|   a.v = 2
|   Node b = Node()
|   b.v = 3
|   Node root = Node()
|   root.kids = [a, b]
|   echo(toString_int(len(root.kids)))
|   echo(toString_int(sum(root)))
|   return 0
