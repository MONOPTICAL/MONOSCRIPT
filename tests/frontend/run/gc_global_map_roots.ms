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
// map живут только в глобальных переменных
map<string,string> m0 = {"k": "keep" + 1000}
map<string,string> m1 = {"k": "keep" + 1001}
map<string,string> m2 = {"k": "keep" + 1002}
map<string,string> m3 = {"k": "keep" + 1003}
map<string,string> m4 = {"k": "keep" + 1004}
map<string,string> m5 = {"k": "keep" + 1005}
map<string,string> m6 = {"k": "keep" + 1006}
map<string,string> m7 = {"k": "keep" + 1007}
map<string,string> m8 = {"k": "keep" + 1008}
map<string,string> m9 = {"k": "keep" + 1009}
map<string,string> m10 = {"k": "keep" + 1010}
map<string,string> m11 = {"k": "keep" + 1011}
map<string,string> m12 = {"k": "keep" + 1012}
map<string,string> m13 = {"k": "keep" + 1013}
map<string,string> m14 = {"k": "keep" + 1014}
map<string,string> m15 = {"k": "keep" + 1015}
map<string,string> m16 = {"k": "keep" + 1016}
map<string,string> m17 = {"k": "keep" + 1017}
map<string,string> m18 = {"k": "keep" + 1018}
map<string,string> m19 = {"k": "keep" + 1019}
map<string,string> m20 = {"k": "keep" + 1020}
map<string,string> m21 = {"k": "keep" + 1021}
map<string,string> m22 = {"k": "keep" + 1022}
map<string,string> m23 = {"k": "keep" + 1023}
map<string,string> m24 = {"k": "keep" + 1024}
map<string,string> m25 = {"k": "keep" + 1025}
map<string,string> m26 = {"k": "keep" + 1026}
map<string,string> m27 = {"k": "keep" + 1027}
map<string,string> m28 = {"k": "keep" + 1028}
map<string,string> m29 = {"k": "keep" + 1029}
map<string,string> m30 = {"k": "keep" + 1030}
map<string,string> m31 = {"k": "keep" + 1031}
map<string,string> m32 = {"k": "keep" + 1032}
map<string,string> m33 = {"k": "keep" + 1033}
map<string,string> m34 = {"k": "keep" + 1034}
map<string,string> m35 = {"k": "keep" + 1035}
map<string,string> m36 = {"k": "keep" + 1036}
map<string,string> m37 = {"k": "keep" + 1037}
map<string,string> m38 = {"k": "keep" + 1038}
map<string,string> m39 = {"k": "keep" + 1039}
map<string,string> m40 = {"k": "keep" + 1040}
map<string,string> m41 = {"k": "keep" + 1041}
map<string,string> m42 = {"k": "keep" + 1042}
map<string,string> m43 = {"k": "keep" + 1043}
map<string,string> m44 = {"k": "keep" + 1044}
map<string,string> m45 = {"k": "keep" + 1045}
map<string,string> m46 = {"k": "keep" + 1046}
map<string,string> m47 = {"k": "keep" + 1047}
map<string,string> m48 = {"k": "keep" + 1048}
map<string,string> m49 = {"k": "keep" + 1049}
map<string,string> m50 = {"k": "keep" + 1050}
map<string,string> m51 = {"k": "keep" + 1051}
map<string,string> m52 = {"k": "keep" + 1052}
map<string,string> m53 = {"k": "keep" + 1053}
map<string,string> m54 = {"k": "keep" + 1054}
map<string,string> m55 = {"k": "keep" + 1055}
map<string,string> m56 = {"k": "keep" + 1056}
map<string,string> m57 = {"k": "keep" + 1057}
map<string,string> m58 = {"k": "keep" + 1058}
map<string,string> m59 = {"k": "keep" + 1059}
map<string,string> m60 = {"k": "keep" + 1060}
map<string,string> m61 = {"k": "keep" + 1061}
map<string,string> m62 = {"k": "keep" + 1062}
map<string,string> m63 = {"k": "keep" + 1063}

[void]churn()
|   i32 i = 0
|   while i < 100000
|   |   map<string,string> junk = {"k": "kill" + i}
|   |   i = i + 1

[void]check()
|   echo(m0["k"])
|   echo(m1["k"])
|   echo(m2["k"])
|   echo(m3["k"])
|   echo(m4["k"])
|   echo(m5["k"])
|   echo(m6["k"])
|   echo(m7["k"])
|   echo(m8["k"])
|   echo(m9["k"])
|   echo(m10["k"])
|   echo(m11["k"])
|   echo(m12["k"])
|   echo(m13["k"])
|   echo(m14["k"])
|   echo(m15["k"])
|   echo(m16["k"])
|   echo(m17["k"])
|   echo(m18["k"])
|   echo(m19["k"])
|   echo(m20["k"])
|   echo(m21["k"])
|   echo(m22["k"])
|   echo(m23["k"])
|   echo(m24["k"])
|   echo(m25["k"])
|   echo(m26["k"])
|   echo(m27["k"])
|   echo(m28["k"])
|   echo(m29["k"])
|   echo(m30["k"])
|   echo(m31["k"])
|   echo(m32["k"])
|   echo(m33["k"])
|   echo(m34["k"])
|   echo(m35["k"])
|   echo(m36["k"])
|   echo(m37["k"])
|   echo(m38["k"])
|   echo(m39["k"])
|   echo(m40["k"])
|   echo(m41["k"])
|   echo(m42["k"])
|   echo(m43["k"])
|   echo(m44["k"])
|   echo(m45["k"])
|   echo(m46["k"])
|   echo(m47["k"])
|   echo(m48["k"])
|   echo(m49["k"])
|   echo(m50["k"])
|   echo(m51["k"])
|   echo(m52["k"])
|   echo(m53["k"])
|   echo(m54["k"])
|   echo(m55["k"])
|   echo(m56["k"])
|   echo(m57["k"])
|   echo(m58["k"])
|   echo(m59["k"])
|   echo(m60["k"])
|   echo(m61["k"])
|   echo(m62["k"])
|   echo(m63["k"])

[i32]main()
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   check()
|   return 0
