# Squawk: An AWK Language Interpreter

It's still under development, but it builds --- there's no parsing though yet. You can build `squawk.c` with:

```
gcc squawk.c -lunistring -lgc -lpcre2-posix
```

You will need these 3 dependences:

1- PCRE2

2- Boehm GC

3- GNU Libunistring

If you wish to generate `squawk.gperf` which is the LUT for builtin functions, you need gperf.

Pretty soon VMGEN will be added to the mix.

Plan on using Re2C for lexing and Yacc for parsing.

Please lemme know about any ideas that you have for Squawk:

```
chubakbidpaa@gmail.com
chubakbidpaa@riseup.net
```

Thanks!
