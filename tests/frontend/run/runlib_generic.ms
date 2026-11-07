[T]libFirst<T>(array<T>: a)
|   return libHelp(a[0])

[T]libHelp<T>(T: x)
|   return x

[T]libPriv<T>(T: x) @private
|   return x
