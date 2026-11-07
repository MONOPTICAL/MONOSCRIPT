// expect-runtime-error: index 5 out of bounds (len 1) at line 3
[T]at<T>(array<T>: a)
|   return a[5]

[i32]main()
|   echo(cstr(at([1])))
|   return 0
