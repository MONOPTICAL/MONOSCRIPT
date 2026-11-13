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
// expect-output: keep1032
// expect-output: keep1033
// expect-output: keep1034
// expect-output: keep1035
// expect-output: keep1036
// expect-output: keep1037
// expect-output: keep1038
// expect-output: keep1039
// expect-output: keep1040
// expect-output: keep1041
// expect-output: keep1042
// expect-output: keep1043
// expect-output: keep1044
// expect-output: keep1045
// expect-output: keep1046
// expect-output: keep1047
// expect-output: keep1048
// expect-output: keep1049
// expect-output: keep1050
// expect-output: keep1051
// expect-output: keep1052
// expect-output: keep1053
// expect-output: keep1054
// expect-output: keep1055
// expect-output: keep1056
// expect-output: keep1057
// expect-output: keep1058
// expect-output: keep1059
// expect-output: keep1060
// expect-output: keep1061
// expect-output: keep1062
// expect-output: keep1063
// Строки из кучи живут только в глобальных переменных. Сборщик не видит память JIT сам,
// без GC_add_roots почти все они пропадают. Переменных много, чтобы не спасал случайный указатель на стеке
string s0 = "keep" + 1000
string s1 = "keep" + 1001
string s2 = "keep" + 1002
string s3 = "keep" + 1003
string s4 = "keep" + 1004
string s5 = "keep" + 1005
string s6 = "keep" + 1006
string s7 = "keep" + 1007
string s8 = "keep" + 1008
string s9 = "keep" + 1009
string s10 = "keep" + 1010
string s11 = "keep" + 1011
string s12 = "keep" + 1012
string s13 = "keep" + 1013
string s14 = "keep" + 1014
string s15 = "keep" + 1015
string s16 = "keep" + 1016
string s17 = "keep" + 1017
string s18 = "keep" + 1018
string s19 = "keep" + 1019
string s20 = "keep" + 1020
string s21 = "keep" + 1021
string s22 = "keep" + 1022
string s23 = "keep" + 1023
string s24 = "keep" + 1024
string s25 = "keep" + 1025
string s26 = "keep" + 1026
string s27 = "keep" + 1027
string s28 = "keep" + 1028
string s29 = "keep" + 1029
string s30 = "keep" + 1030
string s31 = "keep" + 1031
string s32 = "keep" + 1032
string s33 = "keep" + 1033
string s34 = "keep" + 1034
string s35 = "keep" + 1035
string s36 = "keep" + 1036
string s37 = "keep" + 1037
string s38 = "keep" + 1038
string s39 = "keep" + 1039
string s40 = "keep" + 1040
string s41 = "keep" + 1041
string s42 = "keep" + 1042
string s43 = "keep" + 1043
string s44 = "keep" + 1044
string s45 = "keep" + 1045
string s46 = "keep" + 1046
string s47 = "keep" + 1047
string s48 = "keep" + 1048
string s49 = "keep" + 1049
string s50 = "keep" + 1050
string s51 = "keep" + 1051
string s52 = "keep" + 1052
string s53 = "keep" + 1053
string s54 = "keep" + 1054
string s55 = "keep" + 1055
string s56 = "keep" + 1056
string s57 = "keep" + 1057
string s58 = "keep" + 1058
string s59 = "keep" + 1059
string s60 = "keep" + 1060
string s61 = "keep" + 1061
string s62 = "keep" + 1062
string s63 = "keep" + 1063

[void]churn()
|   i32 i = 0
|   while i < 300000
|   |   string junk = "kill" + i
|   |   i = i + 1

[void]check()
|   echo(s0)
|   echo(s1)
|   echo(s2)
|   echo(s3)
|   echo(s4)
|   echo(s5)
|   echo(s6)
|   echo(s7)
|   echo(s8)
|   echo(s9)
|   echo(s10)
|   echo(s11)
|   echo(s12)
|   echo(s13)
|   echo(s14)
|   echo(s15)
|   echo(s16)
|   echo(s17)
|   echo(s18)
|   echo(s19)
|   echo(s20)
|   echo(s21)
|   echo(s22)
|   echo(s23)
|   echo(s24)
|   echo(s25)
|   echo(s26)
|   echo(s27)
|   echo(s28)
|   echo(s29)
|   echo(s30)
|   echo(s31)
|   echo(s32)
|   echo(s33)
|   echo(s34)
|   echo(s35)
|   echo(s36)
|   echo(s37)
|   echo(s38)
|   echo(s39)
|   echo(s40)
|   echo(s41)
|   echo(s42)
|   echo(s43)
|   echo(s44)
|   echo(s45)
|   echo(s46)
|   echo(s47)
|   echo(s48)
|   echo(s49)
|   echo(s50)
|   echo(s51)
|   echo(s52)
|   echo(s53)
|   echo(s54)
|   echo(s55)
|   echo(s56)
|   echo(s57)
|   echo(s58)
|   echo(s59)
|   echo(s60)
|   echo(s61)
|   echo(s62)
|   echo(s63)

[i32]main()
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   check()
|   return 0
