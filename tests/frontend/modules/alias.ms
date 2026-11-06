// expect: ok
// xfail: алиас импорта не применяется, Linker.cpp
use
|> lib -> pub : hh

[i32]main()
|   return hh()
