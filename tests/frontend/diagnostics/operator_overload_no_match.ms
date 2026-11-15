// expect-error: No overload of V.op_mul accepts
[struct]V
|   i32 x = 0
|   [V]operator*(i32: k) @pure
|   |   return V()
|   [V]operator*(V: other) @pure
|   |   return V()

[i32]main()
|   V v = V()
|   V w = v * "a"
|   return 0
