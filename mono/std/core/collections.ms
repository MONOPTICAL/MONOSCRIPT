[struct]pair<T, Q>
|   T first
|   Q second
|   
|   [void]init(T: f, Q: s)
|   |    self.first = f
|   |    self.second = s

[struct]set<T>
|   array<T> elements
|
|   [void]init()
|   |   self.elements = []
|   
|   [void]add(T: element)
|   |   if !self.contains(element)
|   |   |   push(self.elements, element)
|
|   [bool]contains(T: element)
|   |   for e in self.elements
|   |   |   if e == element
|   |   |   |   return true
|   |   return false
|   
|   [void]remove(T: element)
|   |   array<T> newElements = []
|   |   for e in self.elements
|   |   |   if e != element
|   |   |   |   push(newElements, e)
|   |   self.elements = newElements