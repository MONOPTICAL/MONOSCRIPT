// expect-output: 200
// expect-output: 1200
// expect-output: 3232235777
// expect-output: 192
// expect-output: 1
// expect-output: 323223577
// expect-output: 777
// expect-output: big
// expect-output: 255
// expect-output: 31 144
// expect-output: 9223372036854775808
// expect-output: 18446744073709551615
// expect-output: 3232235777.0
// expect-output: 0
// expect-output: 200
// expect-output: 2147483648
[i32]main()
|   u8 b = 200
|   echo(cstr(b))
|   i32 x = b + 1000
|   echo(cstr(x))
|   u32 ip = 3232235777
|   echo(cstr(ip))
|   echo(cstr(ip shr 24))
|   echo(cstr((ip shr 8)->u8))
|   echo(cstr(ip / 10))
|   echo(cstr(ip % 1000))
|   if ip > 3000000000
|   |   echo("big")
|   i32 neg = 0 - 1
|   u8 w = neg->u8
|   echo(cstr(w))
|   u16 port = 8080
|   u8 hi = (port shr 8)->u8
|   u8 lo = (port band 255)->u8
|   echo(cstr(hi) + " " + cstr(lo))
|   u64 big = 1
|   u64 m = big shl 63
|   echo(cstr(m))
|   echo(cstr(bnot 0->u64))
|   f64 f = ip
|   echo(cstr(f))
|   u8 c = 255
|   u8 d = c + 1
|   echo(cstr(d))
|   array<u8> bytes = [1, 200, 255]
|   echo(cstr(bytes[1]))
|   echo(cstr(2->u32 ** 31))
|   return 0
