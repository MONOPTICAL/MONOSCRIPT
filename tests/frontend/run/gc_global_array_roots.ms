// expect-output: 2000
// expect-output: 2001
// expect-output: 2002
// expect-output: 2003
// expect-output: 2004
// expect-output: 2005
// expect-output: 2006
// expect-output: 2007
// expect-output: 2008
// expect-output: 2009
// expect-output: 2010
// expect-output: 2011
// expect-output: 2012
// expect-output: 2013
// expect-output: 2014
// expect-output: 2015
// expect-output: 2016
// expect-output: 2017
// expect-output: 2018
// expect-output: 2019
// expect-output: 2020
// expect-output: 2021
// expect-output: 2022
// expect-output: 2023
// expect-output: 2024
// expect-output: 2025
// expect-output: 2026
// expect-output: 2027
// expect-output: 2028
// expect-output: 2029
// expect-output: 2030
// expect-output: 2031
// expect-output: keep1000
// expect-output: keep1001
// expect-output: keep1002
// expect-output: keep1003
// expect-output: keep1004
// expect-output: keep1005
// expect-output: keep1006
// expect-output: keep1007
// expect-output: keep1008
// expect-output: keep1009
// expect-output: keep1010
// expect-output: keep1011
// expect-output: keep1012
// expect-output: keep1013
// expect-output: keep1014
// expect-output: keep1015
// expect-output: keep1016
// expect-output: keep1017
// expect-output: keep1018
// expect-output: keep1019
// expect-output: keep1020
// expect-output: keep1021
// expect-output: keep1022
// expect-output: keep1023
// expect-output: keep1024
// expect-output: keep1025
// expect-output: keep1026
// expect-output: keep1027
// expect-output: keep1028
// expect-output: keep1029
// expect-output: keep1030
// expect-output: keep1031
// Массивы живут только в глобальных переменных: и заголовок, и данные, и строки в них.
// Без GC_add_roots сборщик в JIT их не видит. Переменных много, чтобы не спасал случайный указатель на стеке
array<i32> n0 = [2000, 7]
array<i32> n1 = [2001, 7]
array<i32> n2 = [2002, 7]
array<i32> n3 = [2003, 7]
array<i32> n4 = [2004, 7]
array<i32> n5 = [2005, 7]
array<i32> n6 = [2006, 7]
array<i32> n7 = [2007, 7]
array<i32> n8 = [2008, 7]
array<i32> n9 = [2009, 7]
array<i32> n10 = [2010, 7]
array<i32> n11 = [2011, 7]
array<i32> n12 = [2012, 7]
array<i32> n13 = [2013, 7]
array<i32> n14 = [2014, 7]
array<i32> n15 = [2015, 7]
array<i32> n16 = [2016, 7]
array<i32> n17 = [2017, 7]
array<i32> n18 = [2018, 7]
array<i32> n19 = [2019, 7]
array<i32> n20 = [2020, 7]
array<i32> n21 = [2021, 7]
array<i32> n22 = [2022, 7]
array<i32> n23 = [2023, 7]
array<i32> n24 = [2024, 7]
array<i32> n25 = [2025, 7]
array<i32> n26 = [2026, 7]
array<i32> n27 = [2027, 7]
array<i32> n28 = [2028, 7]
array<i32> n29 = [2029, 7]
array<i32> n30 = [2030, 7]
array<i32> n31 = [2031, 7]
array<string> s0 = ["keep" + 1000, "x"]
array<string> s1 = ["keep" + 1001, "x"]
array<string> s2 = ["keep" + 1002, "x"]
array<string> s3 = ["keep" + 1003, "x"]
array<string> s4 = ["keep" + 1004, "x"]
array<string> s5 = ["keep" + 1005, "x"]
array<string> s6 = ["keep" + 1006, "x"]
array<string> s7 = ["keep" + 1007, "x"]
array<string> s8 = ["keep" + 1008, "x"]
array<string> s9 = ["keep" + 1009, "x"]
array<string> s10 = ["keep" + 1010, "x"]
array<string> s11 = ["keep" + 1011, "x"]
array<string> s12 = ["keep" + 1012, "x"]
array<string> s13 = ["keep" + 1013, "x"]
array<string> s14 = ["keep" + 1014, "x"]
array<string> s15 = ["keep" + 1015, "x"]
array<string> s16 = ["keep" + 1016, "x"]
array<string> s17 = ["keep" + 1017, "x"]
array<string> s18 = ["keep" + 1018, "x"]
array<string> s19 = ["keep" + 1019, "x"]
array<string> s20 = ["keep" + 1020, "x"]
array<string> s21 = ["keep" + 1021, "x"]
array<string> s22 = ["keep" + 1022, "x"]
array<string> s23 = ["keep" + 1023, "x"]
array<string> s24 = ["keep" + 1024, "x"]
array<string> s25 = ["keep" + 1025, "x"]
array<string> s26 = ["keep" + 1026, "x"]
array<string> s27 = ["keep" + 1027, "x"]
array<string> s28 = ["keep" + 1028, "x"]
array<string> s29 = ["keep" + 1029, "x"]
array<string> s30 = ["keep" + 1030, "x"]
array<string> s31 = ["keep" + 1031, "x"]

[void]churn()
|   i32 i = 0
|   while i < 300000
|   |   array<i32> a = [-1, -1]
|   |   array<string> b = ["kill" + i, "y"]
|   |   i = i + 1

[void]check()
|   echo(toString_int(n0[0]))
|   echo(toString_int(n1[0]))
|   echo(toString_int(n2[0]))
|   echo(toString_int(n3[0]))
|   echo(toString_int(n4[0]))
|   echo(toString_int(n5[0]))
|   echo(toString_int(n6[0]))
|   echo(toString_int(n7[0]))
|   echo(toString_int(n8[0]))
|   echo(toString_int(n9[0]))
|   echo(toString_int(n10[0]))
|   echo(toString_int(n11[0]))
|   echo(toString_int(n12[0]))
|   echo(toString_int(n13[0]))
|   echo(toString_int(n14[0]))
|   echo(toString_int(n15[0]))
|   echo(toString_int(n16[0]))
|   echo(toString_int(n17[0]))
|   echo(toString_int(n18[0]))
|   echo(toString_int(n19[0]))
|   echo(toString_int(n20[0]))
|   echo(toString_int(n21[0]))
|   echo(toString_int(n22[0]))
|   echo(toString_int(n23[0]))
|   echo(toString_int(n24[0]))
|   echo(toString_int(n25[0]))
|   echo(toString_int(n26[0]))
|   echo(toString_int(n27[0]))
|   echo(toString_int(n28[0]))
|   echo(toString_int(n29[0]))
|   echo(toString_int(n30[0]))
|   echo(toString_int(n31[0]))
|   echo(s0[0])
|   echo(s1[0])
|   echo(s2[0])
|   echo(s3[0])
|   echo(s4[0])
|   echo(s5[0])
|   echo(s6[0])
|   echo(s7[0])
|   echo(s8[0])
|   echo(s9[0])
|   echo(s10[0])
|   echo(s11[0])
|   echo(s12[0])
|   echo(s13[0])
|   echo(s14[0])
|   echo(s15[0])
|   echo(s16[0])
|   echo(s17[0])
|   echo(s18[0])
|   echo(s19[0])
|   echo(s20[0])
|   echo(s21[0])
|   echo(s22[0])
|   echo(s23[0])
|   echo(s24[0])
|   echo(s25[0])
|   echo(s26[0])
|   echo(s27[0])
|   echo(s28[0])
|   echo(s29[0])
|   echo(s30[0])
|   echo(s31[0])

[i32]main()
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   check()
|   return 0
