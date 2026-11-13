// expect-output: =2
// expect-output: =1
// expect-output: =1
// Как в Python: вхождения не перекрываются, "aaaa".count("aa") == 2.
// "ababab"/"aba" тоже 1: после совпадения в 0 остаётся "bab", второе "aba" начинается внутри первого.
[i32]main()
|   echo("=" + count("aaaa", "aa"))
|   echo("=" + count("aaa", "aa"))
|   echo("=" + count("ababab", "aba"))
|   return 0
