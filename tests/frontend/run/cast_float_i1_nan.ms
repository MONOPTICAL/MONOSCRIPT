// expect-runtime-error: float value out of range for i1 at line 5
[i32]main()
|   float z = 0.0
|   float n = z / z
|   i1 b = n->i1
|   return 0
